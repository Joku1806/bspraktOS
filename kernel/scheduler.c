#include <arch/bsp/systimer.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <stddef.h>

void schedule_thread() {
  node *current_running = get_thread_list_head(running);
  node *ready_head = get_thread_list_head(ready);

  if (ready_head != NULL) {
    if (current_running != NULL) {
      transfer_thread_block_to_list(current_running, ready_head->previous);
    }

    transfer_thread_block_to_list(ready_head, get_thread_list_head(running));
    systimer_reset();
    load_thread_context((tcb *)ready_head);
  }

  // wenn kein Thread ready ist, dann hier warten
  for (;;) {
    asm volatile("WFI");
  }
}