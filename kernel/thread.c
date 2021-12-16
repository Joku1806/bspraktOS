#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Thread"

#include <arch/bsp/stack_defines.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <arch/cpu/registers.h>
#include <kernel/scheduler.h>
#include <kernel/syscall.h>
#include <kernel/thread.h>
#include <lib/assertions.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <lib/modmath.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>

static tcb blocks[THREAD_COUNT];

node ready_head = {.previous = NULL, .next = NULL};
node waiting_head = {.previous = NULL, .next = NULL};
node running_head = {.previous = NULL, .next = NULL};
node finished_head = {.previous = NULL, .next = (node *)blocks};

node *get_idle_thread() { return (node *)&blocks[IDLE_THREAD_INDEX]; }

node *get_thread_list_head(thread_status status) {
  switch (status) {
    case ready:
      return &ready_head;
    case waiting:
      return &waiting_head;
    case running:
      return &running_head;
    case finished:
      return &finished_head;
  }

  VERIFY_NOT_REACHED();
}

const char *get_list_name(node *head) {
  if (head == &ready_head) {
    return "Ready List";
  } else if (head == &waiting_head) {
    return "Waiting List";
  } else if (head == &running_head) {
    return "Running List";
  } else if (head == &finished_head) {
    return "Finished List";
  }

  VERIFY_NOT_REACHED();
}

size_t get_thread_id(node *n) { return ((tcb *)n)->index; }

void reset_thread_context(size_t index) {
  blocks[index].index = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs.sp = (void *)(THREAD_SP_BASE - index * STACK_SIZE);
  blocks[index].regs.lr = sys$exit;
  blocks[index].regs.pc = index == IDLE_THREAD_INDEX ? halt_cpu : NULL;
}

void thread_list_initialise() {
  for (size_t i = 0; i < USER_THREAD_COUNT; i++) {
    node *current = (node *)&blocks[i];
    current->previous = (node *)&blocks[MODULO_SUB(i, 1, USER_THREAD_COUNT)];
    current->next = (node *)&blocks[MODULO_ADD(i, 1, USER_THREAD_COUNT)];
    reset_thread_context(i);
  }

  node *idle_thread = get_idle_thread();
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
  node *ready_head = get_thread_list_head(ready);
  node *finished_head = get_thread_list_head(finished);

  if (is_list_empty(finished_head)) {
    return;
  }

  node *tnode = get_first_node(finished_head);
  tcb *thread = (tcb *)tnode;
  dbgln("Assigning new task to thread %u.", get_thread_id(tnode));

  thread->regs.sp = (char *)thread->regs.sp - args_size;
  if (args_size % 8 != 0) { // 8-byte align
    thread->regs.sp = (char *)thread->regs.sp - (8 - args_size % 8);
  }

  memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;
  thread->regs.pc = func;

  if (is_list_empty(ready_head)) {
    transfer_list_node(finished_head, ready_head, tnode);
  } else {
    transfer_list_node(finished_head, get_last_node(ready_head), tnode);
  }
}

void thread_cleanup() {
  node *running_head = get_thread_list_head(running);
  node *finished_head = get_thread_list_head(finished);
  node *me = get_first_node(get_thread_list_head(running));

  dbgln("Exiting from thread %u", get_thread_id(me));
  reset_thread_context(get_thread_id(me));
  transfer_list_node(running_head, finished_head, me);
}