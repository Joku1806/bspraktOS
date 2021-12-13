#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Bounded Linked List"

#include <lib/assertions.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <stddef.h>

bool is_list_head(node *n) { return n != NULL && n->previous == NULL; }

bool is_list_node(node *n) {
  return n != NULL && n->previous != NULL && n->next != NULL;
}

bool is_list_empty(node *list) {
  VERIFY(is_list_head(list));
  return list->next == NULL;
}

node *get_first_node(node *list) {
  VERIFY(is_list_head(list));
  return list->next == NULL ? list : list->next;
}

node *get_last_node(node *list) {
  VERIFY(is_list_head(list));
  return list->next == NULL ? list : list->next->previous;
}

void connect_nodes(node *from, node *to) {
  VERIFY(from != NULL);
  VERIFY(to != NULL);

  from->next = to;
  to->previous = from;
}

void remove_node_from_list(node *n, node *list) {
  VERIFY(is_list_node(n));
  VERIFY(is_list_head(list));

  if (list->next == n) {
    if (n == n->previous && n == n->next) {
      dbgln("n = %p is first and only node in list.", n);
      list->next = NULL;
      return;
    }

    list->next = n->next;
  }

  connect_nodes(n->previous, n->next);
  connect_nodes(n, n);
}

// FIXME: list muss nicht unbedingt ein list head sein
void append_node_to_list(node *n, node *list) {
  VERIFY(is_list_node(n));
  VERIFY(is_list_node(list) || is_list_head(list));

  if (is_list_head(list)) {
    if (list->next != NULL) {
      connect_nodes(n, list->next);
    }

    list->next = n;
  } else {
    connect_nodes(n, list->next);
    connect_nodes(list, n);
  }
}