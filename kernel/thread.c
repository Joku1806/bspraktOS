#include <arch/bsp/stack_defines.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <arch/cpu/registers.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/assertions.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <user/main.h>

extern void schedule_thread();
static tcb blocks[USER_THREAD_COUNT];

node *ready_head = NULL;
node *waiting_head = NULL;
node *running_head = NULL;
node *finished_head = (node *)&blocks[0];

void reset_thread_context(size_t index) {
  blocks[index].index = index;
  blocks[index].cpsr = psr_mode_user;
  blocks[index].regs[SP_POSITION] = THREAD_SP_BASE - index * STACK_SIZE;
  blocks[index].regs[LR_POSITION] = (uint32_t)thread_cleanup;
  blocks[index].regs[PC_POSITION] = (uint32_t)main;
}

void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr) {
  for (size_t index = 0; index < 16; index++) {
    thread->regs[index] = regs[index];
  }

  thread->cpsr = cpsr;
}

void load_thread_context(tcb *thread) {
  asm volatile("msr cpsr, %0 \n\t"
               "ldmfd %1, {r0-r15} \n\t" ::"r"(thread->cpsr),
               "r"(thread->regs)
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
  memcpy(thread->regs[SP_POSITION], args, args_size);

  remove_node_from_current_list((node *)thread);
  append_node_to_list((node *)thread,
                      ready_head == NULL ? &ready_head : &ready_head->previous);
}

void thread_cleanup() {
  node *me = running_head;
  reset_thread_context(((tcb *)me)->index);
  remove_node_from_current_list(me);
  append_node_to_list(me, &finished_head);
  // FIXME: sollte nicht gemacht werden. Optimal wäre wenn wir dafür einen
  // syscall schreiben der dann in einem privilegierten Modus alles macht.
  schedule_thread(((tcb *)me)->regs);
}

void thread_list_initialise() {
  for (size_t i = 0; i < USER_THREAD_COUNT; i++) {
    node *current = (node *)&blocks[i];
    // FIXME: vllt Macro für sowas?
    if (i > 0) {
      current->previous = (node *)&blocks[i - 1];
    } else {
      current->previous = (node *)&blocks[USER_THREAD_COUNT - 1];
    }

    if (i < USER_THREAD_COUNT - 1) {
      current->next = (node *)&blocks[i + 1];
    } else {
      current->next = (node *)&blocks[0];
    }

    reset_thread_context(i);
  }

  idle_thread.index = IDLE_THREAD_INDEX;
  idle_thread.cpsr = psr_mode_user;
  idle_thread.regs[SP_POSITION] =
      THREAD_SP_BASE - IDLE_THREAD_INDEX * STACK_SIZE;
  idle_thread.regs[LR_POSITION] = (uint32_t)thread_cleanup;
  idle_thread.regs[PC_POSITION] = (uint32_t)halt_cpu;
}

void transfer_thread_block_to_list(node *tcb_node, node *list) {
  VERIFY(tcb_node != NULL && list != NULL);

  if (tcb_node->previous != NULL) {
    tcb_node->previous->next = tcb_node->next;
  }

  if (tcb_node->next != NULL) {
    tcb_node->next->previous = tcb_node->previous;
  }

  if (list == NULL) {
    list = tcb_node;
    tcb_node->previous = tcb_node;
    tcb_node->next = tcb_node;
  } else {
    tcb_node->previous = list;
    tcb_node->next = list->next;

    list->next->previous = tcb_node;
    list->next = tcb_node;
  }
}

void thread_cleanup() {
  node *me = get_thread_list_head(running);
  reset_thread_context(((tcb *)me)->index);
  transfer_thread_block_to_list(me, get_thread_list_head(finished));
  schedule_thread();
}