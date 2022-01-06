#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Intrusive List"

#define VERIFY_INTEGRITY_AFTER_OP false

#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/intrusive_list.h>
#include <stdbool.h>
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

bool is_first_node(node *list, node *n) {
  if (is_list_empty(list)) {
    return false;
  }

  return get_first_node(list) == n;
}

bool is_last_node(node *list, node *n) {
  if (is_list_empty(list)) {
    return false;
  }

  return get_last_node(list) == n;
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
  dbgln("Removing node %p from list %p.", n, p);

  if (list->next == n) {
    dbgln("Node %p is first node in list.", n);
    // FIXME: wird der erste Check überhaupt gebraucht?
    if (n == n->previous && n == n->next) {
      dbgln("Node %p is also only node in list, clearing list head.", n);
      list->next = NULL;
    } else {
      dbgln("Pointing head of list %p to second node %p.", list, n->next);
      list->next = n->next;
    }
  }

  connect_nodes(n->previous, n->next);
  connect_nodes(n, n);
}

// FIXME: list muss nicht unbedingt ein list head sein
void append_node_to_list(node *list, node *n) {
  VERIFY(is_list_node(list) || is_list_head(list));
  VERIFY(is_list_node(n));
  VERIFY(list != n);

  if (is_list_head(list)) {
    if (list->next == NULL) {
      list->next = n;
      return;
    }

    // am Ende der Liste anhängen ist das gleiche wie als erstes Element in die Liste einfügen
    node *head = list;
    list = get_last_node(list);
    dbgln("Pointing head of list %p to node %p.", head, n);
    head->next = n;
  }

  connect_nodes(n, list->next);
  connect_nodes(list, n);
}

void transfer_list_node(node *from, node *to, node *n) {
  remove_node_from_list(from, n);
  append_node_to_list(to, n);
}