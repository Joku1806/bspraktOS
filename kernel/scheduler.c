#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Scheduler"

#include <arch/cpu/psr.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <stddef.h>
#include <stdint.h>

void schedule_thread(registers *thread_regs) {
  node *running_list = get_thread_list_head(running);
  node *ready_list = get_thread_list_head(ready);
  node *next_thread = NULL;

  if (is_list_empty(ready_list) && !is_list_empty(running_list)) {
    dbgln("No other thread waiting for work, continuing to run current thread.");
    return;
  }

  if (is_list_empty(ready_list) && is_list_empty(running_list)) {
    dbgln("No thread waiting for work at all, scheduling idle thread (id=%u).", get_thread_id(get_idle_thread()));
    next_thread = get_idle_thread();
  } else if (!is_list_empty(ready_list)) {
    dbgln("Now scheduling thread %u.", get_thread_id(get_first_node(ready_list)));

    if (!is_list_empty(running_list)) {
      node *current_thread = get_first_node(running_list);
      remove_node_from_list(get_thread_list_head(running), current_thread);

      if (current_thread != get_idle_thread()) {
        dbgln("Thread %u is not done yet, saving context.", get_thread_id(current_thread));
        append_node_to_list(get_last_node(ready_list), current_thread);
        save_thread_context((tcb *)current_thread, thread_regs, get_spsr());
      }

      verify_linked_list_integrity();
    }

    next_thread = get_first_node(ready_list);
  }

  // FIXME: Ziemlich verwirrend, dass next_thread nicht unbedingt Teil der ready
  // Liste sein muss, um aus seiner aktuellen Liste entfernt zu werden.
  remove_node_from_list(get_thread_list_head(ready), next_thread);
  append_node_to_list(get_thread_list_head(running), next_thread);
  verify_linked_list_integrity();
  perform_stack_context_switch(thread_regs, (tcb *)next_thread);
  kprintf("\n");
}