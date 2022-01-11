#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/bsp/stack_defines.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <arch/cpu/registers.h>
#include <kernel/scheduler.h>
#include <kernel/syscall.h>
#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/intrusive_list.h>
#include <lib/math.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

static tcb blocks[THREAD_COUNT];

node ready_list = {.previous = NULL, .next = NULL};
node stall_waiting_list = {.previous = NULL, .next = NULL};
node input_waiting_list = {.previous = NULL, .next = NULL};
node running_list = {.previous = NULL, .next = NULL};
node finished_list = {.previous = NULL, .next = (node *)blocks};

node *scheduler_get_idle_thread() { return (node *)&blocks[IDLE_THREAD_INDEX]; }
tcb *scheduler_get_running_thread() {
  VERIFY(!is_list_empty(&running_list));
  return (tcb *)get_first_node(&running_list);
}

size_t get_thread_id(node *n) {
  return ((tcb *)n)->id;
}

void reset_thread_context(size_t index) {
  blocks[index].id = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs.sp = (void *)(THREAD_SP_BASE - index * STACK_SIZE);
  blocks[index].regs.lr = sys$exit_thread;
  blocks[index].regs.pc = index == IDLE_THREAD_INDEX ? halt_cpu : NULL;
}

void thread_list_initialise() {
  for (size_t i = 0; i < USER_THREAD_COUNT; i++) {
    node *current = (node *)&blocks[i];
    current->previous = (node *)&blocks[MODULO_SUB(i, 1, USER_THREAD_COUNT)];
    current->next = (node *)&blocks[MODULO_ADD(i, 1, USER_THREAD_COUNT)];
    reset_thread_context(i);
  }

  node *idle_thread = scheduler_get_idle_thread();
  idle_thread->previous = idle_thread;
  idle_thread->next = idle_thread;
  reset_thread_context(IDLE_THREAD_INDEX);
}

void save_thread_context(tcb *thread, registers *regs) {
  memcpy(&thread->regs.general, regs->general, sizeof(thread->regs.general));

  // Thread $sp und $lr separat speichern, weil es am Anfang
  // der Interruptbehandlung überschrieben wurde und deswegen
  // nur im Usermodus verfügbar ist.
  asm volatile("mrs %0, sp_usr \n\t"
               "mrs %1, lr_usr \n\t"
               : "=r"(thread->regs.sp), "=r"(thread->regs.lr)::"memory");

  // $pc muss mit $lr überschrieben werden, weil
  // der eigentliche $pc nach dem Interrupt dort steht.
  thread->regs.pc = regs->lr;
}

void perform_stack_context_switch(registers *current_thread_regs, tcb *thread) {
  // generelle Register sowie lr(_irq) mit unserer Startfunktion
  // überschreiben, weil am Ende des Interrupthandlers pc auf lr(_irq) gesetzt
  // wird.
  memcpy(&current_thread_regs->general, (void *)thread->regs.general, sizeof(thread->regs.general));
  current_thread_regs->lr = thread->regs.pc;

  // Usermode in spsr schreiben, damit am Ende des Interrupthandlers durch
  // movs in den Usermodus gewechselt wird. Da sp und lr gebankt sind und wir
  // hier noch im IRQ Modus sind, müssen sie auch explizit überschrieben
  // werden.
  asm volatile("msr spsr_cxsf, %0 \n\t"
               "msr sp_usr, %1 \n\t"
               "msr lr_usr, %2 \n\t" ::"r"(thread->cpsr),
               "r"(thread->regs.sp), "r"(thread->regs.lr)
               : "memory");
}

void thread_create(void (*func)(void *), const void *args, unsigned int args_size) {
  VERIFY(!is_list_empty(&finished_list));

  node *tnode = get_first_node(&finished_list);
  tcb *thread = (tcb *)tnode;
  dbgln("Assigning new task to thread %u.", get_thread_id(tnode));

  thread->regs.sp -= align8(args_size);
  memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;
  thread->regs.pc = func;

  if (is_list_empty(&ready_list)) {
    transfer_list_node(&finished_list, &ready_list, tnode);
  } else {
    transfer_list_node(&finished_list, get_last_node(&ready_list), tnode);
  }
}

void thread_cleanup() {
  node *me = get_first_node(&running_list);

  dbgln("Exiting from thread %u", get_thread_id(me));
  reset_thread_context(get_thread_id(me));
  transfer_list_node(&running_list, &finished_list, me);
}

bool scheduler_is_thread_available() {
  return !is_list_empty(&finished_list);
}

#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
void scheduler_verify_thread_list_integrity() {
  dbgln("Now checking list integrity.");

  VERIFY(&stall_waiting_list != &running_list && &stall_waiting_list != &ready_list && &stall_waiting_list != &input_waiting_list && &stall_waiting_list != &finished_list);
  VERIFY(&running_list != &ready_list && &running_list != &input_waiting_list && &running_list != &finished_list);
  VERIFY(&ready_list != &input_waiting_list && &ready_list != &finished_list);
  VERIFY(&input_waiting_list != &finished_list);

  char *list_names[5] = {"running list", "ready list", "waiting list (input)", "waiting list (stall)", "finished list"};
  node *lists[5] = {&running_list, &ready_list, &input_waiting_list, &stall_waiting_list, &finished_list};
  bool checked[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; i++) {
    checked[i] = false;
  }

  for (size_t i = 0; i < 5; i++) {
    if (is_list_empty(lists[i])) {
      dbgln("%s is empty, continuing...", get_list_name(lists[i]));
      continue;
    }

    node *start = get_first_node(lists[i]);
    node *current = start;

    do {
      dbgln("Now checking thread %u, currently part of %s.", ((tcb *)current)->id, list_names[i]);
      dbgln("%u <-> %u <-> %u", ((tcb *)current->previous)->id, ((tcb *)current)->id, ((tcb *)current->next)->id);
      VERIFY(is_list_node(current));
      VERIFY(current == current->previous->next);
      VERIFY(current == current->next->previous);
      VERIFY(!checked[get_thread_id(current)]);
      checked[get_thread_id(current)] = true;
      current = current->next;
    } while (current != start);
  }

  dbgln("List is in a valid state. Congratulations!");
}
#endif

void scheduler_ignore_thread_until_character_input(tcb *thread) {

  // Wir gehen davon aus, dass vor dieser Funktion schedule_thread()
  // aufgerufen wurde und thread deswegen jetzt in der ready Liste ist
  if (is_list_empty(&input_waiting_list)) {
    transfer_list_node(&ready_list, &input_waiting_list, (node *)thread);
  } else {
    transfer_list_node(&ready_list, get_last_node(&input_waiting_list), (node *)thread);
  }
}

void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match) {

  thread->stall_until = match;
  // Wir gehen davon aus, dass vor dieser Funktion schedule_thread()
  // aufgerufen wurde und thread deswegen jetzt in der ready Liste ist
  if (is_list_empty(&stall_waiting_list)) {
    transfer_list_node(&ready_list, &stall_waiting_list, (node *)thread);
  } else {
    transfer_list_node(&ready_list, get_last_node(&stall_waiting_list), (node *)thread);
  }
}

void scheduler_unblock_input_waiting_threads(char ch) {
  while (!is_list_empty(&input_waiting_list)) {
    node *current = get_first_node(&input_waiting_list);
    tcb *thread = (tcb *)current;

    *(char *)thread->regs.sp = ch;

    // FIXME: Weg finden dieses Pattern nicht mehr benutzen zu müssen
    // Außerdem: soll append oder prepend gemacht werden?
    if (is_list_empty(&ready_list)) {
      transfer_list_node(&input_waiting_list, &ready_list, current);
    } else {
      transfer_list_node(&input_waiting_list, get_last_node(&ready_list), current);
    }
  }
}

void scheduler_unblock_stall_waiting_threads(unsigned current_time) {

  if (is_list_empty(&stall_waiting_list)) {
    return;
  }

  node *current = get_first_node(&stall_waiting_list);
  do {
    tcb *thread = (tcb *)current;

    if (thread->stall_until <= current_time) {
      if (is_list_empty(&ready_list)) {
        transfer_list_node(&stall_waiting_list, &ready_list, current);
      } else {
        transfer_list_node(&stall_waiting_list, get_last_node(&ready_list), current);
      }
    }

    current = current->next;
  } while (!is_first_node(&stall_waiting_list, current));
}

void schedule_thread(registers *thread_regs) {
  node *next_thread = NULL;

  if (is_list_empty(&ready_list) && !is_list_empty(&running_list)) {
    dbgln("No other thread waiting for work, continuing to run current thread.");
    return;
  }

  // FIXME: idle thread vielleicht einfach in ready Liste einfügen,
  // sollte Logik ein bisschen vereinfachen
  if (is_list_empty(&ready_list) && is_list_empty(&running_list)) {
    dbgln("No thread waiting for work at all, scheduling idle thread (id=%u).", get_thread_id(get_idle_thread()));
    next_thread = scheduler_get_idle_thread();
  } else if (!is_list_empty(&ready_list)) {
    dbgln("Now scheduling thread %u.", get_thread_id(get_first_node(ready_head)));

    if (!is_list_empty(&running_list)) {
      node *running_thread = get_first_node(&running_list);
      remove_node_from_list(&running_list, running_thread);

      if (running_thread != scheduler_get_idle_thread()) {
        dbgln("Thread %u is not done yet, saving context.", get_thread_id(current_thread));
        append_node_to_list(get_last_node(&ready_list), running_thread);
        save_thread_context((tcb *)running_thread, thread_regs);
      }
    }

    next_thread = get_first_node(&ready_list);
  }

  // FIXME: Ziemlich verwirrend, dass next_thread nicht unbedingt Teil der ready
  // Liste sein muss, um aus seiner aktuellen Liste entfernt zu werden.
  transfer_list_node(&ready_list, &running_list, next_thread);
  perform_stack_context_switch(thread_regs, (tcb *)next_thread);
  kprintf("\n");
}