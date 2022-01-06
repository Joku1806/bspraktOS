#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Scheduler"

#include <kernel/scheduler.h>
#include <lib/debug.h>
#include <lib/intrusive_list.h>
#include <stddef.h>

bool is_thread_available() {
  return !is_list_empty(get_thread_list_head(finished));
}

void scheduler_ignore_thread_until_character_input(tcb *thread) {
  node *i_waiting_list = get_thread_list_head(input_waiting);
  node *ready_list = get_thread_list_head(ready);

  // Wir gehen davon aus, dass vor dieser Funktion schedule_thread()
  // aufgerufen wurde und thread deswegen jetzt in der ready Liste ist
  if (is_list_empty(i_waiting_list)) {
    transfer_list_node(ready_list, i_waiting_list, (node *)thread);
  } else {
    transfer_list_node(ready_list, get_last_node(i_waiting_list), (node *)thread);
  }
}

void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match) {
  node *s_waiting_list = get_thread_list_head(stall_waiting);
  node *ready_list = get_thread_list_head(ready);

  thread->stall_until = match;
  // Wir gehen davon aus, dass vor dieser Funktion schedule_thread()
  // aufgerufen wurde und thread deswegen jetzt in der ready Liste ist
  if (is_list_empty(s_waiting_list)) {
    transfer_list_node(ready_list, s_waiting_list, (node *)thread);
  } else {
    transfer_list_node(ready_list, get_last_node(s_waiting_list), (node *)thread);
  }
}

void scheduler_unblock_input_waiting_threads(char ch) {
  node *i_waiting_list = get_thread_list_head(input_waiting);
  node *ready_list = get_thread_list_head(ready);

  while (!is_list_empty(i_waiting_list)) {
    node *current = get_first_node(i_waiting_list);
    tcb *thread = (tcb *)current;

    *(char *)thread->regs.sp = ch;

    // FIXME: Weg finden dieses Pattern nicht mehr benutzen zu müssen
    // Außerdem: soll append oder prepend gemacht werden?
    if (is_list_empty(ready_list)) {
      transfer_list_node(i_waiting_list, ready_list, current);
    } else {
      transfer_list_node(i_waiting_list, get_last_node(ready_list), current);
    }
  }
}

void scheduler_unblock_stall_waiting_threads(unsigned current_time) {
  node *s_waiting_list = get_thread_list_head(stall_waiting);
  node *ready_list = get_thread_list_head(ready);

  if (is_list_empty(s_waiting_list)) {
    return;
  }

  node *current = get_first_node(s_waiting_list);
  do {
    tcb *thread = (tcb *)current;

    if (thread->stall_until <= current_time) {
      if (is_list_empty(ready_list)) {
        transfer_list_node(s_waiting_list, ready_list, current);
      } else {
        transfer_list_node(s_waiting_list, get_last_node(ready_list), current);
      }
    }

    current = current->next;
  } while (!is_first_node(s_waiting_list, current));
}

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