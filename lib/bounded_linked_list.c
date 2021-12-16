
#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Bounded Linked List"

#include <kernel/thread.h>
#include <lib/assertions.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <stddef.h>

bool is_list_head(node *n) {
  VERIFY(n != NULL);
  return n->previous == NULL;
}

bool is_list_node(node *n) {
  VERIFY(n != NULL);
  return n->previous != NULL && n->next != NULL;
}

bool is_list_empty(node *list) {
  VERIFY(is_list_head(list));
  return list->next == NULL;
}

node *get_first_node(node *list) {
  VERIFY(is_list_head(list));
  VERIFY(!is_list_empty(list));
  VERIFY(is_list_node(list->next));
  return list->next;
}

node *get_last_node(node *list) {
  VERIFY(is_list_head(list));
  VERIFY(!is_list_empty(list));
  VERIFY(is_list_node(list->next->previous));
  return list->next->previous;
}

void verify_linked_list_integrity() {
  dbgln("Now checking list integrity.");
  node *ruh = get_thread_list_head(running);
  node *reh = get_thread_list_head(ready);
  node *wh = get_thread_list_head(waiting);
  node *fh = get_thread_list_head(finished);

  VERIFY(ruh != reh && ruh != wh && ruh != fh);
  VERIFY(reh != wh && reh != fh);
  VERIFY(wh != fh);

  node *lists[4] = {ruh, reh, wh, fh};
  bool checked[THREAD_COUNT] = {false};

  for (size_t i = 0; i < 4; i++) {
    if (is_list_empty(lists[i])) {
      dbgln("%s is empty, continuing...", get_list_name(lists[i]));
      continue;
    }

    node *start = get_first_node(lists[i]);
    node *current = start;

    do {
      dbgln("Now checking thread %u, currently part of %s.", get_thread_id(current), get_list_name(lists[i]));
      dbgln("%u <-> %u <-> %u", get_thread_id(current->previous), get_thread_id(current), get_thread_id(current->next));
      VERIFY(is_list_node(current));
      VERIFY(current == current->previous->next);
      VERIFY(current == current->next->previous);
      VERIFY(!checked[get_thread_id(current)]);
      checked[get_thread_id(current)] = true;
      current = current->next;
    } while (current != start);
  }

  dbgln("List is in a valid state. Congratulations!");
}

void connect_nodes(node *from, node *to) {
  VERIFY(is_list_node(from));
  VERIFY(is_list_node(to));
  dbgln("Connecting node %u to node %u.", get_thread_id(from), get_thread_id(to));

  from->next = to;
  to->previous = from;
}

void remove_node_from_list(node *list, node *n) {
  VERIFY(is_list_head(list));
  VERIFY(is_list_node(n));
  dbgln("Removing node %u from %s.", get_thread_id(n), get_list_name(list));

  if (list->next == n) {
    dbgln("Node %u is first node in list.", get_thread_id(n));
    // FIXME: wird der erste Check überhaupt gebraucht?
    if (n == n->previous && n == n->next) {
      dbgln("Node %u is also only node in list, clearing list head.", get_thread_id(n));
      list->next = NULL;
    } else {
      dbgln("Pointing %s head to second node %u.", get_list_name(list), get_thread_id(n->next));
      list->next = n->next;
    }
  }

  connect_nodes(n->previous, n->next);
  connect_nodes(n, n);

  verify_linked_list_integrity();
}

// FIXME: list muss nicht unbedingt ein list head sein
void append_node_to_list(node *list, node *n) {
  VERIFY(is_list_node(list) || is_list_head(list));
  VERIFY(is_list_node(n));
  VERIFY(list != n);

  if (is_list_head(list)) {
    if (list->next != NULL) {
      dbgln("%s is not empty, connecting node %u with first list node %u.", get_list_name(list), get_thread_id(n), get_thread_id(list->next));
      connect_nodes(n, list->next);
      connect_nodes(get_last_node(list), n);
    }

    dbgln("Pointing %s head to node %u.", get_list_name(list), get_thread_id(n));
    list->next = n;
  } else {
    dbgln("Inserting node %u between node %u and node %u.", get_thread_id(n), get_thread_id(list), get_thread_id(list->next));
    connect_nodes(n, list->next);
    connect_nodes(list, n);
  }

  verify_linked_list_integrity();
}

void transfer_list_node(node *from, node *to, node *n) {
  remove_node_from_list(from, n);
  append_node_to_list(to, n);
}