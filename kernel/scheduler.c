#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Scheduler"

#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <stddef.h>
#include <stdint.h>

void schedule_thread(registers *thread_regs) {
  node *ready_head = get_thread_list_head(ready);
  node *running_head = get_thread_list_head(running);
  node *next_thread = NULL;

  if (is_list_empty(ready_head) && !is_list_empty(running_head)) {
    dbgln("No other thread waiting for work, continuing to run current thread.");
    return;
  }

  // FIXME: idle thread vielleicht einfach in ready Liste einfügen,
  // sollte Logik ein bisschen vereinfachen
  if (is_list_empty(ready_head) && is_list_empty(running_head)) {
    dbgln("No thread waiting for work at all, scheduling idle thread (id=%u).", get_thread_id(get_idle_thread()));
    next_thread = get_idle_thread();
  } else if (!is_list_empty(ready_head)) {
    dbgln("Now scheduling thread %u.", get_thread_id(get_first_node(ready_head)));

    if (!is_list_empty(running_head)) {
      node *current_thread = get_first_node(running_head);
      remove_node_from_list(running_head, current_thread);

      if (current_thread != get_idle_thread()) {
        dbgln("Thread %u is not done yet, saving context.", get_thread_id(current_thread));
        append_node_to_list(get_last_node(ready_head), current_thread);
        save_thread_context((tcb *)current_thread, thread_regs);
      }
    }

    next_thread = get_first_node(ready_head);
  }

  // FIXME: Ziemlich verwirrend, dass next_thread nicht unbedingt Teil der ready
  // Liste sein muss, um aus seiner aktuellen Liste entfernt zu werden.
  transfer_list_node(ready_head, running_head, next_thread);
  perform_stack_context_switch(thread_regs, (tcb *)next_thread);
  kprintf("\n");
}