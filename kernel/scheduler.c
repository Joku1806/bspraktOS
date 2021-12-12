#include "lib/bounded_linked_list.h"
#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/cpu/psr.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/debug.h>
#include <stddef.h>
#include <stdint.h>

void schedule_thread(uint32_t *thread_regs) {
  node *running_list = get_thread_list_head(running);
  node *ready_list = get_thread_list_head(ready);
  node *next_thread = NULL;

  if (is_list_empty(ready_list) && !is_list_empty(running_list)) {
    dbgln(
        "No other thread waiting for work, continuing to run current thread.");
    return;
  }

  if (is_list_empty(ready_list) && is_list_empty(running_list)) {
    dbgln("No thread waiting for work at all, scheduling idle thread.");
    next_thread = (node *)get_idle_thread();
  } else if (!is_list_empty(ready_list)) {
    dbgln("Now scheduling thread %u.",
          ((tcb *)get_first_node(ready_list))->index);
    if (!is_list_empty(running_list)) {
      node *current_thread = get_first_node(running_list);
      dbgln("Thread %u is not done yet, saving context.",
            ((tcb *)current_thread)->index);
      save_thread_context((tcb *)current_thread, thread_regs, get_spsr());
      remove_node_from_list(current_thread, get_thread_list_head(running));
      append_node_to_list(current_thread, get_last_node(ready_list));
    }

    next_thread = get_first_node(ready_list);
  }

  dbgln("Now switching out old thread for thread %u.",
        ((tcb *)next_thread)->index);
  // FIXME: Ziemlich verwirrend, dass next_thread nicht unbedingt Teil der ready
  // Liste sein muss, um aus seiner aktuellen Liste entfernt zu werden.
  remove_node_from_list(next_thread, get_thread_list_head(ready));
  append_node_to_list(next_thread, get_thread_list_head(running));
  perform_stack_context_switch(thread_regs, (tcb *)next_thread);
}