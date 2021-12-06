#include <kernel/bounded_thread_list.h>
#include <kernel/scheduler.h>
#include <stddef.h>

void schedule_next_thread() {
  node *current_running = get_thread_list_head(running);
  node *ready_head = get_thread_list_head(ready);

  if (ready_head != NULL) {
    if (current_running != NULL) {
      transfer_node_to_list(current_running, ready_head->previous);
    }

    // Zeitscheibe zurücksetzen
    switch_thread_context(ready_head);
    transfer_node_to_list(ready_head, get_thread_list_head(running));
  }
}