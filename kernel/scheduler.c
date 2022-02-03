#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/bsp/memory_map.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <arch/cpu/registers.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kintrusive_list.h>
#include <kernel/lib/kmath.h>
#include <kernel/lib/kstring.h>
#include <kernel/scheduler.h>
#include <stddef.h>
#include <stdint.h>

bool scheduler_stall_cmp(k_node *a, k_node *b);

__attribute__((weak)) void user_main() {
  kpanicln("Could not link to real user_main().");
}

__attribute__((weak)) void sys$exit_thread() {
  kpanicln("Could not link to real sys$exit_thread().");
}

static tcb blocks[THREAD_COUNT];

k_node ready_list = {.previous = NULL, .next = (k_node *)&blocks[0]};
k_node stall_waiting_list = {.previous = NULL, .next = NULL};
k_node input_waiting_list = {.previous = NULL, .next = NULL};
k_node running_list = {.previous = NULL, .next = NULL};
k_node finished_list = {.previous = NULL, .next = (k_node *)&blocks[1]};

k_node *scheduler_get_idle_thread() { return (k_node *)&blocks[IDLE_THREAD_INDEX]; }
tcb *scheduler_get_running_thread() {
  VERIFY(!k_is_list_empty(&running_list));
  return (tcb *)k_get_first_node(&running_list);
}

size_t scheduler_get_thread_id(k_node *n) {
  return ((tcb *)n)->id;
}

void scheduler_reset_thread_context(size_t index) {
  blocks[index].id = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs.sp = (void *)(USER_STACK_TOP_ADDRESS - index * STACK_SIZE);
  blocks[index].regs.lr = sys$exit_thread;
  blocks[index].regs.pc = NULL;
}

void scheduler_reset_idle_thread_context() {
  blocks[IDLE_THREAD_INDEX].id = IDLE_THREAD_INDEX;
  blocks[IDLE_THREAD_INDEX].cpsr = psr_mode_user;
  blocks[IDLE_THREAD_INDEX].regs.sp = (void *)(USER_STACK_TOP_ADDRESS - IDLE_THREAD_INDEX * STACK_SIZE);
  blocks[IDLE_THREAD_INDEX].regs.lr = halt_cpu;
  blocks[IDLE_THREAD_INDEX].regs.pc = halt_cpu;
}

void scheduler_initialise() {
  k_node *first_running_thread = (k_node *)&blocks[0];
  first_running_thread->previous = first_running_thread;
  first_running_thread->next = first_running_thread;
  scheduler_reset_thread_context(0);
  ((tcb *)first_running_thread)->regs.pc = user_main;

  for (size_t i = 1; i < USER_THREAD_COUNT; i++) {
    k_node *current = (k_node *)&blocks[i];
    current->previous = (k_node *)&blocks[k_modulo_sub(i, 1, 1, USER_THREAD_COUNT)];
    current->next = (k_node *)&blocks[k_modulo_add(i, 1, 1, USER_THREAD_COUNT)];
    scheduler_reset_thread_context(i);
  }

  k_node *idle_thread = scheduler_get_idle_thread();
  idle_thread->previous = idle_thread;
  idle_thread->next = idle_thread;
  scheduler_reset_idle_thread_context();
}

void scheduler_save_thread_context(tcb *thread, registers *regs) {
  k_memcpy(&thread->regs.general, regs->general, sizeof(thread->regs.general));

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

void scheduler_switch_thread(registers *current_thread_regs, tcb *thread) {
  // generelle Register sowie lr(_irq) mit unserer Startfunktion
  // überschreiben, weil am Ende des Interrupthandlers pc auf lr(_irq) gesetzt
  // wird.
  k_memcpy(&current_thread_regs->general, (void *)thread->regs.general, sizeof(thread->regs.general));
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

void scheduler_create_thread(void (*func)(void *), const void *args, unsigned int args_size) {
  VERIFY(!k_is_list_empty(&finished_list));

  k_node *tnode = k_get_first_node(&finished_list);
  tcb *thread = (tcb *)tnode;
  kdbgln("Assigning new task to thread %u.", scheduler_get_thread_id(tnode));

  thread->regs.sp -= k_align8(args_size);
  k_memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;
  thread->regs.pc = func;

  if (k_is_list_empty(&ready_list)) {
    k_transfer_list_node(&finished_list, &ready_list, tnode);
  } else {
    k_transfer_list_node(&finished_list, k_get_last_node(&ready_list), tnode);
  }
}

void scheduler_cleanup_thread() {
  k_node *me = k_get_first_node(&running_list);

  kdbgln("Exiting from thread %u", scheduler_get_thread_id(me));
  scheduler_reset_thread_context(scheduler_get_thread_id(me));
  k_transfer_list_node(&running_list, &finished_list, me);
}

bool scheduler_is_thread_available() {
  return !k_is_list_empty(&finished_list);
}

#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
void scheduler_verify_thread_list_integrity() {
  VERIFY(&stall_waiting_list != &running_list && &stall_waiting_list != &ready_list && &stall_waiting_list != &input_waiting_list && &stall_waiting_list != &finished_list);
  VERIFY(&running_list != &ready_list && &running_list != &input_waiting_list && &running_list != &finished_list);
  VERIFY(&ready_list != &input_waiting_list && &ready_list != &finished_list);
  VERIFY(&input_waiting_list != &finished_list);

  k_node *lists[5] = {&running_list, &ready_list, &input_waiting_list, &stall_waiting_list, &finished_list};
  bool checked[THREAD_COUNT];
  for (size_t i = 0; i < THREAD_COUNT; i++) {
    checked[i] = false;
  }

  for (size_t i = 0; i < 5; i++) {
    if (k_is_list_empty(lists[i])) {
      continue;
    }

    k_node *start = k_get_first_node(lists[i]);
    k_node *current = start;

    do {
      VERIFY(k_is_list_node(current));
      VERIFY(current == current->previous->next);
      VERIFY(current == current->next->previous);
      VERIFY(!checked[scheduler_get_thread_id(current)]);
      checked[scheduler_get_thread_id(current)] = true;
      current = current->next;
    } while (current != start);

    if (lists[i] != &stall_waiting_list) {
      continue;
    }

    k_node *current_s = k_get_first_node(lists[i]);
    k_node *last_s = k_get_last_node(lists[i]);

    while (current_s != last_s) {
      VERIFY(scheduler_stall_cmp(current_s, current_s->next));
      current_s = current_s->next;
    }
  }
}
#endif

void scheduler_ignore_thread_until_character_input(tcb *thread) {
  // Wir gehen davon aus, dass vor dieser Funktion schedule_thread()
  // aufgerufen wurde und thread deswegen jetzt in der ready Liste ist
  if (k_is_list_empty(&input_waiting_list)) {
    k_transfer_list_node(&ready_list, &input_waiting_list, (k_node *)thread);
  } else {
    k_transfer_list_node(&ready_list, k_get_last_node(&input_waiting_list), (k_node *)thread);
  }
}

bool scheduler_stall_cmp(k_node *a, k_node *b) {
  return ((tcb *)a)->stall_until <= ((tcb *)b)->stall_until;
}

void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match) {
  k_remove_node_from_list(&ready_list, (k_node *)thread);

  thread->stall_until = systimer_value() + match;
  k_insert_sorted(&stall_waiting_list, (k_node *)thread, scheduler_stall_cmp);

  if ((k_node *)thread == k_get_first_node(&stall_waiting_list) && scheduler_adjust_stall_timer() < 0) {
    // FIXME: Falls Wert zu klein, wird er einfach in die ready Liste verschoben,
    // sollte man in dem Fall einen Fehler zurückgeben?
    kwarnln("Could not stall thread %u because systimer already passed interrupt time.", thread->id);
    scheduler_unblock_overdue_waiting_threads();
  }
}

bool scheduler_exists_input_waiting_thread() {
  return !k_is_list_empty(&input_waiting_list);
}

void scheduler_unblock_first_input_waiting_thread(char ch) {
  k_node *first = k_get_first_node(&input_waiting_list);
  tcb *thread = (tcb *)first;

  thread->regs.general[0] = ch;

  // FIXME: Weg finden dieses Pattern nicht mehr benutzen zu müssen
  // Außerdem: soll append oder prepend gemacht werden?
  if (k_is_list_empty(&ready_list)) {
    k_transfer_list_node(&input_waiting_list, &ready_list, first);
  } else {
    k_transfer_list_node(&input_waiting_list, k_get_last_node(&ready_list), first);
  }
}

void scheduler_unblock_overdue_waiting_threads() {
  while (scheduler_adjust_stall_timer() < 0) {
    k_node *first = k_get_first_node(&stall_waiting_list);
    kdbgln("Thread %u is overdue.", scheduler_get_thread_id(first));

    if (k_is_list_empty(&ready_list)) {
      k_transfer_list_node(&stall_waiting_list, &ready_list, first);
    } else {
      k_transfer_list_node(&stall_waiting_list, k_get_last_node(&ready_list), first);
    }
  }
}

int scheduler_adjust_stall_timer() {
  if (k_is_list_empty(&stall_waiting_list)) {
    stalltimer_reset_pending_interrupt();
    return 0;
  }

  k_node *first = k_get_first_node(&stall_waiting_list);
  return stalltimer_interrupt_at(((tcb *)first)->stall_until);
}

void scheduler_round_robin(registers *thread_regs) {
  if (k_is_list_empty(&ready_list) && !k_is_list_empty(&running_list)) {
    kdbgln("No other thread waiting for work, continuing to run current thread.");
    return;
  }

  if (k_is_list_empty(&ready_list) && k_is_list_empty(&running_list)) {
    k_append_node_to_list(&ready_list, scheduler_get_idle_thread());
  }

  kdbgln("Now scheduling thread %u.", scheduler_get_thread_id(k_get_first_node(&ready_list)));
  if (!k_is_list_empty(&running_list)) {
    k_node *running_thread = k_get_first_node(&running_list);
    k_remove_node_from_list(&running_list, running_thread);

    if (running_thread != scheduler_get_idle_thread()) {
      kdbgln("Thread %u is not done yet, saving context.", scheduler_get_thread_id(running_thread));
      k_append_node_to_list(k_get_last_node(&ready_list), running_thread);
      scheduler_save_thread_context((tcb *)running_thread, thread_regs);
    }
  }

  k_node *next_thread = k_get_first_node(&ready_list);
  k_transfer_list_node(&ready_list, &running_list, next_thread);
  scheduler_switch_thread(thread_regs, (tcb *)next_thread);

#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
  scheduler_verify_thread_list_integrity();
#endif
}

void scheduler_forced_round_robin(registers *thread_regs) {
  if (k_is_list_empty(&ready_list)) {
    k_append_node_to_list(&ready_list, scheduler_get_idle_thread());
  }

  kdbgln("Now scheduling thread %u.", scheduler_get_thread_id(k_get_first_node(&ready_list)));
  if (!k_is_list_empty(&running_list)) {
    k_node *running_thread = k_get_first_node(&running_list);
    k_remove_node_from_list(&running_list, running_thread);

    if (running_thread != scheduler_get_idle_thread()) {
      kdbgln("Thread %u is not done yet, saving context.", scheduler_get_thread_id(running_thread));
      k_append_node_to_list(k_get_last_node(&ready_list), running_thread);
      scheduler_save_thread_context((tcb *)running_thread, thread_regs);
    }
  }

  k_node *next_thread = k_get_first_node(&ready_list);
  k_transfer_list_node(&ready_list, &running_list, next_thread);
  scheduler_switch_thread(thread_regs, (tcb *)next_thread);

#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
  scheduler_verify_thread_list_integrity();
#endif
}