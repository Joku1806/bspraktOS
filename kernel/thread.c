#define LOG_LEVEL DEBUG_LEVEL
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

static tcb blocks[USER_THREAD_COUNT];
static tcb idle_thread;

node ready_head = {.previous = NULL, .next = NULL};
node waiting_head = {.previous = NULL, .next = NULL};
node running_head = {.previous = NULL, .next = NULL};
node finished_head = {.previous = NULL, .next = (node *)blocks};

node *get_idle_thread() { return (node *)&idle_thread; }

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
  blocks[index].regs.pc = NULL;
}

void idle_thread_initialize() {
  node *idle_thread_node = (node *)&idle_thread;
  idle_thread_node->previous = idle_thread_node;
  idle_thread_node->next = idle_thread_node;

  idle_thread.index = IDLE_THREAD_INDEX;
  idle_thread.cpsr = psr_mode_user;
  idle_thread.regs.sp = (void *)(THREAD_SP_BASE - IDLE_THREAD_INDEX * STACK_SIZE);
  idle_thread.regs.lr = sys$exit;
  idle_thread.regs.pc = halt_cpu;
}

void thread_list_initialise() {
  for (size_t i = 0; i < USER_THREAD_COUNT; i++) {
    node *current = (node *)&blocks[i];
    current->previous = (node *)&blocks[MODULO_SUB(i, 1, USER_THREAD_COUNT)];
    current->next = (node *)&blocks[MODULO_ADD(i, 1, USER_THREAD_COUNT)];
    reset_thread_context(i);
  }

  idle_thread_initialize();
}

void save_thread_context(tcb *thread, registers *regs, uint32_t cpsr) {
  memcpy(&thread->regs, regs, sizeof(thread->regs));
  thread->cpsr = cpsr;
}

void perform_stack_context_switch(registers *current_thread_regs, tcb *thread) {
  // generelle Register sowie lr(_irq) mit unserer Startfunktion
  // überschreiben, weil am Ende des Interrupthandlers pc auf lr(_irq) gesetzt
  // wird.
  memcpy(current_thread_regs, (void *)&thread->regs, sizeof(thread->regs.general));
  current_thread_regs->lr = thread->regs.pc;

  // Usermode in spsr schreiben, damit am Ende des Interrupthandlers durch
  // movs in den Usermodus gewechselt wird. Da sp und lr gebankt sind und wir
  // hier noch im IRQ Modus sind, müssen sie auch explizit überschrieben
  // werden.
  asm volatile("msr spsr_cxsf, %0 \n\t"
               "msr sp_usr, %1 \n\t"
               "msr lr_usr, %2 \n\t" ::"I"(psr_mode_user),
               "r"(thread->regs.sp), "r"(thread->regs.lr)
               : "memory");
}

void thread_create(void (*func)(void *), const void *args, unsigned int args_size) {
  if (is_list_empty(get_thread_list_head(finished))) {
    return;
  }

  tcb *thread = (tcb *)get_first_node(get_thread_list_head(finished));
  dbgln("Assigning new task to thread %u.", get_thread_id((node *)thread));

  thread->regs.sp = (char *)thread->regs.sp - args_size;
  if (args_size % 8 != 0) { // 8-byte align
    thread->regs.sp = (char *)thread->regs.sp - (8 - args_size % 8);
  }

  memcpy(thread->regs.sp, args, args_size);
  thread->regs.general[0] = (uint32_t)thread->regs.sp;
  thread->regs.pc = func;

  remove_node_from_list(get_thread_list_head(finished), (node *)thread);
  append_node_to_list(get_last_node(get_thread_list_head(ready)), (node *)thread);
  verify_linked_list_integrity();
}

void thread_cleanup() {
  node *me = get_first_node(get_thread_list_head(running));
  VERIFY(is_list_node(me));

  dbgln("Exiting from thread %u", get_thread_id(me));
  reset_thread_context(((tcb *)me)->index);
  remove_node_from_list(get_thread_list_head(running), me);
  append_node_to_list(get_thread_list_head(finished), me);
  verify_linked_list_integrity();
}