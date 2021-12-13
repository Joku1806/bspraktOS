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

tcb *get_idle_thread() { return &idle_thread; }

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

void reset_thread_context(size_t index) {
  blocks[index].index = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs[SP_POSITION] = THREAD_SP_BASE - index * STACK_SIZE;
  blocks[index].regs[LR_POSITION] = (uint32_t)sys$exit;
  blocks[index].regs[PC_POSITION] = (uint32_t)NULL;
}

void idle_thread_initialize() {
  node *idle_thread_node = (node *)&idle_thread;
  idle_thread_node->previous = idle_thread_node;
  idle_thread_node->next = idle_thread_node;

  idle_thread.index = IDLE_THREAD_INDEX;
  idle_thread.cpsr = psr_mode_user;
  idle_thread.regs[SP_POSITION] =
      THREAD_SP_BASE - IDLE_THREAD_INDEX * STACK_SIZE;
  idle_thread.regs[LR_POSITION] = (uint32_t)sys$exit;
  idle_thread.regs[PC_POSITION] = (uint32_t)halt_cpu;
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

void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr) {
  for (size_t index = 0; index < 16; index++) {
    thread->regs[index] = regs[index];
  }

  thread->cpsr = cpsr;
}

void perform_stack_context_switch(uint32_t *current_thread_regs, tcb *thread) {
  // generelle Register sowie lr(_irq) mit unserer Startfunktion
  // überschreiben, weil am Ende des Interrupthandlers pc auf lr(_irq) gesetzt
  // wird.
  memcpy(current_thread_regs, thread->regs, 13 * 4);
  current_thread_regs[LR_POSITION] = thread->regs[PC_POSITION];

  // Usermode in spsr schreiben, damit am Ende des Interrupthandlers durch
  // movs in den Usermodus gewechselt wird. Da sp und lr gebankt sind und wir
  // hier noch im IRQ Modus sind, müssen sie auch explizit überschrieben
  // werden.
  asm volatile("msr spsr_cxsf, %0 \n\t"
               "msr sp_usr, %1 \n\t"
               "msr lr_usr, %2 \n\t" ::"I"(psr_mode_user),
               "r"(thread->regs[SP_POSITION]), "r"(thread->regs[LR_POSITION])
               : "memory");
}

void thread_create(void (*func)(void *), const void *args,
                   unsigned int args_size) {
  if (is_list_empty(get_thread_list_head(finished))) {
    return;
  }

  tcb *thread = (tcb *)get_first_node(get_thread_list_head(finished));

  thread->regs[SP_POSITION] -= args_size;
  if (args_size % 8 != 0) { // 8-byte align
    thread->regs[SP_POSITION] -= 8 - args_size % 8;
  }

  memcpy((void *)thread->regs[SP_POSITION], args, args_size);
  thread->regs[PC_POSITION] = (uint32_t)func;

  remove_node_from_list((node *)thread, get_thread_list_head(finished));
  append_node_to_list((node *)thread,
                      get_last_node(get_thread_list_head(ready)));
}

void thread_cleanup() {
  node *me = get_first_node(get_thread_list_head(running));
  VERIFY(is_list_node(me));

  reset_thread_context(((tcb *)me)->index);
  remove_node_from_list(me, get_thread_list_head(running));
  append_node_to_list(me, get_thread_list_head(finished));
}