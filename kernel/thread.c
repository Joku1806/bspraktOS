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
#include <lib/modmath.h>
#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>
#include <user/main.h>

static tcb blocks[USER_THREAD_COUNT];
static tcb idle_thread;

node *ready_head = NULL;
node *waiting_head = NULL;
node *running_head = NULL;
node *finished_head = (node *)&blocks[0];

tcb *get_idle_thread() { return &idle_thread; }

void reset_thread_context(size_t index) {
  blocks[index].index = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs[SP_POSITION] = THREAD_SP_BASE - index * STACK_SIZE;
  blocks[index].regs[LR_POSITION] = (uint32_t)sys$exit;
  blocks[index].regs[PC_POSITION] = (uint32_t)main;
}

void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr) {
  for (size_t index = 0; index < 16; index++) {
    thread->regs[index] = regs[index];
  }

  thread->cpsr = cpsr;
}

void load_thread_context(tcb *thread, uint32_t *current_thread_regs) {
  // generelle Register sowie lr(_irq) mit unserer Startfunktion überschreiben,
  // weil am Ende des Interrupthandlers pc auf lr(_irq) gesetzt wird.
  memcpy(current_thread_regs, thread->regs, 13 * 4);
  current_thread_regs[LR_POSITION] = thread->regs[PC_POSITION];

  // Usermode in spsr schreiben, damit am Ende des Interrupthandlers durch movs
  // in den Usermodus gewechselt wird. Da sp und lr gebankt sind und wir hier
  // noch im IRQ Modus sind, müssen sie auch explizit überschrieben werden.
  asm volatile("msr spsr, %0 \n\t"
               "msr sp_usr, %1 \n\t"
               "msr lr_usr, %2 \n\t" ::"I"(psr_mode_user),
               "r"(thread->regs[SP_POSITION]), "r"(thread->regs[LR_POSITION])
               : "memory");
}

void thread_create(void (*func)(void *), const void *args,
                   unsigned int args_size) {
  if (finished_head == NULL) {
    return;
  }

  tcb *thread = (tcb *)&finished_head;

  // 8-byte align
  thread->regs[SP_POSITION] += args_size + (8 % (args_size % 8));
  thread->regs[PC_POSITION] = (uint32_t)func;
  memcpy((void *)thread->regs[SP_POSITION], args, args_size);

  remove_node_from_current_list((node *)thread);
  append_node_to_list((node *)thread,
                      ready_head == NULL ? &ready_head : &ready_head->previous);
}

void thread_cleanup() {
  node *me = running_head;
  reset_thread_context(((tcb *)me)->index);
  remove_node_from_current_list(me);
  append_node_to_list(me, &finished_head);
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
    current->previous = (node *)&blocks[MODULO_ADD(i, 1, USER_THREAD_COUNT)];
    reset_thread_context(i);
  }

  idle_thread_initialize();
}