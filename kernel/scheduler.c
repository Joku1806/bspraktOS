#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/bsp/systimer.h>
#include <arch/cpu/psr.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <stddef.h>
#include <stdint.h>

void schedule_thread(uint32_t *thread_regs) {
  node *running_thread = running_head;
  node *ready_thread = ready_head;
  node *next_running_thread = NULL;

  if (ready_thread == NULL && running_thread != NULL) {
    return;
  }

  if (ready_thread == NULL && running_thread == NULL) {
    next_running_thread = (node *)get_idle_thread();
  } else if (ready_thread != NULL) {
    if (running_thread != NULL) {
      save_thread_context((tcb *)running_thread, thread_regs, get_spsr());
      remove_node_from_current_list(running_thread);
      append_node_to_list(running_thread, &ready_thread->previous);
    }

    next_running_thread = ready_thread;
  }

  remove_node_from_current_list(next_running_thread);
  append_node_to_list(next_running_thread, &running_head);
  systimer_reset();
  load_thread_context((tcb *)next_running_thread, thread_regs);
}