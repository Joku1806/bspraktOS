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
#include <kernel/mmu.h>
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

k_node ready_list = {.previous = NULL, .next = &blocks[0].scheduler_node};
k_node stall_waiting_list = {.previous = NULL, .next = NULL};
k_node input_waiting_list = {.previous = NULL, .next = NULL};
k_node running_list = {.previous = NULL, .next = NULL};
k_node finished_list = {.previous = NULL, .next = &blocks[1].scheduler_node};

static k_node address_spaces[ADDRESS_SPACE_COUNT] = {
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
    {.previous = NULL, .next = NULL},
};

l2_handle process_stack_handles[ADDRESS_SPACE_COUNT][THREADS_PER_ADDRESS_SPACE];
bool slots_used[ADDRESS_SPACE_COUNT][THREADS_PER_ADDRESS_SPACE];

k_node *scheduler_get_address_space_list(size_t address_space) {
  VERIFY(address_space < ADDRESS_SPACE_COUNT);
  return &address_spaces[address_space];
}

bool scheduler_find_first_empty_address_space(size_t *index) {
  for (size_t i = 0; i < ADDRESS_SPACE_COUNT; i++) {
    if (k_is_list_empty(&address_spaces[i])) {
      *index = i;
      return true;
    }
  }

  return false;
}

bool scheduler_find_first_empty_slot_in_address_space(size_t address_space, size_t *index) {
  for (size_t i = 0; i < THREADS_PER_ADDRESS_SPACE; i++) {
    if (!slots_used[address_space][i]) {
      *index = i;
      return true;
    }
  }

  return false;
}

tcb *scheduler_get_idle_thread() {
  return &blocks[IDLE_THREAD_INDEX];
}

tcb *scheduler_get_running_thread() {
  VERIFY(!k_is_list_empty(&running_list));
  return container_of(k_get_first_node(&running_list), tcb, scheduler_node);
}

void scheduler_initialise_idle_thread_context() {
  blocks[IDLE_THREAD_INDEX].pid = ADDRESS_SPACE_COUNT;
  blocks[IDLE_THREAD_INDEX].tid = IDLE_THREAD_INDEX;
  blocks[IDLE_THREAD_INDEX].cpsr = psr_mode_system;
  blocks[IDLE_THREAD_INDEX].regs.sp = (void *)IDLE_THREAD_SP;
  blocks[IDLE_THREAD_INDEX].regs.lr = halt_cpu;
  blocks[IDLE_THREAD_INDEX].regs.pc = halt_cpu;
}

void scheduler_initialise() {
  for (size_t i = 0; i < USER_THREAD_COUNT; i++) {
    k_node *current = &blocks[i].scheduler_node;
    current->previous = &blocks[k_modulo_sub(i, 1, 0, USER_THREAD_COUNT)].scheduler_node;
    current->next = &blocks[k_modulo_add(i, 1, 0, USER_THREAD_COUNT)].scheduler_node;

    tcb *thread = container_of(current, tcb, scheduler_node);
    thread->stack_handle = get_nth_stack_handle(i);
  }

  tcb *idle_thread = scheduler_get_idle_thread();
  idle_thread->scheduler_node.previous = &idle_thread->scheduler_node;
  idle_thread->scheduler_node.next = &idle_thread->scheduler_node;
  scheduler_initialise_idle_thread_context();

  scheduler_create_process(0, user_main, NULL, 0);
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

void scheduler_create_process(size_t address_space, void (*func)(void *), const void *args, unsigned int args_size) {
  VERIFY(!k_is_list_empty(&finished_list));

  void *kernel_wmap = (void *)(UBSS_UDATA_SECTION_START_ADDRESS + (address_space + 1) * MiB);
  void *kernel_rmap = (void *)(UBSS_UDATA_KERNEL_ORIGINAL_MAP_START_ADDRESS);
  k_memcpy(kernel_wmap, kernel_rmap, 1 * MiB);

  k_node *tnode = k_get_first_node(&finished_list);
  tcb *thread = container_of(tnode, tcb, scheduler_node);

  k_append_node_to_list(scheduler_get_address_space_list(address_space), &thread->addrspace_node);
  process_stack_handles[address_space][0] = thread->stack_handle;
  slots_used[address_space][0] = true;

  thread->pid = address_space;
  thread->cpsr = psr_mode_user;
  thread->regs.sp = (void *)(VIRTUAL_PROCESS_STACKS_TOP_ADDRESS);
  thread->regs.lr = sys$exit_thread;
  thread->regs.pc = func;

  thread->regs.sp -= k_align8(args_size);
  k_memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;

  if (k_is_list_empty(&ready_list)) {
    k_transfer_list_node(&finished_list, &ready_list, tnode);
  } else {
    k_transfer_list_node(&finished_list, k_get_last_node(&ready_list), tnode);
  }
}

void scheduler_create_thread(tcb *caller, size_t process_slot, void (*func)(void *), const void *args, unsigned int args_size) {
  VERIFY(!k_is_list_empty(&finished_list));

  k_node *tnode = k_get_first_node(&finished_list);
  tcb *thread = container_of(tnode, tcb, scheduler_node);
  k_append_node_to_list(&address_spaces[caller->pid], &thread->addrspace_node);

  process_stack_handles[caller->pid][process_slot] = thread->stack_handle;
  slots_used[caller->pid][process_slot] = true;

  thread->pid = caller->pid;
  thread->cpsr = psr_mode_user;
  thread->regs.sp = (void *)(VIRTUAL_PROCESS_STACKS_TOP_ADDRESS - process_slot * STACK_SIZE);
  thread->regs.lr = sys$exit_thread;
  thread->regs.pc = func;

  thread->regs.sp -= k_align8(args_size);
  k_memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;

  if (k_is_list_empty(&ready_list)) {
    k_transfer_list_node(&finished_list, &ready_list, tnode);
  } else {
    k_transfer_list_node(&finished_list, k_get_last_node(&ready_list), tnode);
  }
}

void scheduler_cleanup_thread() {
  k_node *me = k_get_first_node(&running_list);
  tcb *thread = container_of(me, tcb, scheduler_node);

  kdbgln("Exiting from thread with pid = %u, tid = %u.", thread->pid, thread->tid);

  slots_used[thread->pid][thread->pid_slot] = false;
  k_remove_node_from_list(scheduler_get_address_space_list(thread->pid), &thread->addrspace_node);
  k_transfer_list_node(&running_list, &finished_list, me);
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
      tcb *current_thread = container_of(current, tcb, scheduler_node);
      VERIFY(!checked[current_thread->tid]);
      checked[current_thread->tid] = true;
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
  return container_of(a, tcb, scheduler_node)->stall_until <= container_of(b, tcb, scheduler_node)->stall_until;
}

void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match) {
  k_remove_node_from_list(&ready_list, (k_node *)thread);

  thread->stall_until = systimer_value() + match;
  k_insert_sorted(&stall_waiting_list, (k_node *)thread, scheduler_stall_cmp);

  if ((k_node *)thread == k_get_first_node(&stall_waiting_list) && scheduler_adjust_stall_timer() < 0) {
    // FIXME: Falls Wert zu klein, wird er einfach in die ready Liste verschoben,
    // sollte man in dem Fall einen Fehler zurückgeben?
    kwarnln("Could not stall thread %u because systimer already passed interrupt time.", thread->tid);
    scheduler_unblock_overdue_waiting_threads();
  }
}

bool scheduler_exists_input_waiting_thread() {
  return !k_is_list_empty(&input_waiting_list);
}

void scheduler_unblock_first_input_waiting_thread(char ch) {
  k_node *first = k_get_first_node(&input_waiting_list);
  tcb *thread = container_of(first, tcb, scheduler_node);

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
  return stalltimer_interrupt_at(container_of(first, tcb, scheduler_node)->stall_until);
}

void scheduler_round_robin(registers *thread_regs, scheduling_type type) {
  if (type != forced && k_is_list_empty(&ready_list) && !k_is_list_empty(&running_list)) {
    kdbgln("No other thread waiting for work, continuing to run current thread.");
    return;
  }

  if (k_is_list_empty(&ready_list) && k_is_list_empty(&running_list)) {
    k_append_node_to_list(&ready_list, &scheduler_get_idle_thread()->scheduler_node);
  }

  tcb *running_thread = NULL;
  tcb *next_thread = container_of(k_get_first_node(&ready_list), tcb, scheduler_node);
  kdbgln("Now scheduling thread with pid = %u, tid = %u.", next_thread->pid, next_thread->tid);
  if (!k_is_list_empty(&running_list)) {
    running_thread = container_of(k_get_first_node(&running_list), tcb, scheduler_node);
    k_remove_node_from_list(&running_list, &running_thread->scheduler_node);

    if (running_thread != scheduler_get_idle_thread()) {
      kdbgln("Thread with pid = %u, tid = %u is not done yet, saving context.", running_thread->pid, running_thread->tid);
      k_append_node_to_list(k_get_last_node(&ready_list), &running_thread->scheduler_node);
      scheduler_save_thread_context(running_thread, thread_regs);
    }
  }

  if (running_thread == NULL || running_thread->pid != next_thread->pid) {
    uint32_t copy_start = UBSS_UDATA_SECTION_START_ADDRESS + (next_thread->pid + 1) * MiB;
    l1_section_set_base_address(&get_l1_entry(UBSS_UDATA_SECTION_START_ADDRESS / MiB)->section, copy_start);

    for (size_t i = 0; i < THREADS_PER_ADDRESS_SPACE; i++) {
      l1_entry entry = {.handle = process_stack_handles[next_thread->pid][i]};
      set_l1_table_entry(VIRTUAL_PROCESS_STACKS_BOTTOM_ADDRESS / MiB + i, entry);
    }
  }

  k_transfer_list_node(&ready_list, &running_list, &next_thread->scheduler_node);
  scheduler_switch_thread(thread_regs, next_thread);

  // TLBIALL schreiben
  asm volatile("mcr p15, 0, r0, c8, c7, 0");

#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
  scheduler_verify_thread_list_integrity();
#endif
}