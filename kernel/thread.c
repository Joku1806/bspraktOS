#include <arch/bsp/stack_defines.h>
#include <arch/cpu/psr.h>
#include <arch/cpu/registers.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/assertions.h>
#include <stddef.h>
#include <stdint.h>
#include <user/user_thread.h>

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
}

node *get_thread_list_head(thread_status status) {
  switch (status) {
    case ready:
      return ready_head;
    case waiting:
      return waiting_head;
    case running:
      return running_head;
    case finished:
      return finished_head;
  }
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