#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/cpu/psr.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/debug.h>
#include <stddef.h>
#include <stdint.h>

void schedule_thread(uint32_t *thread_regs) {
  node *running_thread = running_head;
  node *ready_thread = ready_head;
  node *next_running_thread = NULL;

  if (ready_thread == NULL && running_thread != NULL) {
    dbgln("No other thread waiting for work, continuing to run Thread %u.",
          ((tcb *)running_thread)->index);
    return;
  }

  if (ready_thread == NULL && running_thread == NULL) {
    dbgln("No thread waiting for work at all, scheduling idle thread.");
    next_running_thread = (node *)get_idle_thread();
  } else if (ready_thread != NULL) {
    dbgln("Now scheduling Thread %u.", ((tcb *)ready_thread)->index);
    if (running_thread != NULL) {
      dbgln("Thread %u is not done yet, saving context.",
            ((tcb *)running_thread)->index);
      save_thread_context((tcb *)running_thread, thread_regs, get_spsr());
      remove_node_from_current_list(running_thread);
      append_node_to_list(running_thread, &ready_thread->previous);
    }

    next_running_thread = ready_thread;
  }

  dbgln("Now performing context switch for old Thread %u and new Thread %u.",
        ((tcb *)running_thread)->index, ((tcb *)next_running_thread)->index);
  remove_node_from_current_list(next_running_thread);
  append_node_to_list(next_running_thread, &running_head);
  load_thread_context((tcb *)next_running_thread, thread_regs);
}