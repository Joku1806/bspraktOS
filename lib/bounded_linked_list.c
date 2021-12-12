#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Bounded Linked List"

#include <lib/assertions.h>
#include <lib/bounded_linked_list.h>
#include <lib/debug.h>
#include <stddef.h>

bool is_list_head(node *n) { return n->previous == NULL; }

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

void linked_list_connect(node *a, node *b) {
  VERIFY(a != NULL);
  VERIFY(b != NULL);

  a->next = b;
  b->previous = a;
}

void remove_node_from_list(node *n, node *list) {
  VERIFY(n != NULL);
  VERIFY(n->previous != NULL);
  VERIFY(n->next != NULL);
  VERIFY(list != NULL);
  VERIFY(is_list_head(list));

  if (list->next == n) {
    if (n->previous == n->next) {
      dbgln("n = %p is first node in list.", n);
      list->next = NULL;
      return;
    }

    list->next = n->next;
  }

  linked_list_connect(n->previous, n->next);
  linked_list_connect(n, n);
}

void append_node_to_list(node *n, node *list) {
  VERIFY(n != NULL && list != NULL);

  if (is_list_head(list)) {
    if (list->next != NULL) {
      linked_list_connect(n, list->next);
    }

    list->next = n;
  } else {
    VERIFY(list->next != NULL);

    linked_list_connect(n, list->next);
    linked_list_connect(list, n);
  }
}