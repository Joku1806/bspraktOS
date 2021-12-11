#include <lib/assertions.h>
#include <lib/bounded_linked_list.h>
#include <stddef.h>

// FIXME: Löscht nicht Element, wenn als einziges in Liste
void remove_node_from_current_list(node *n) {
  VERIFY(n != NULL);
  VERIFY(n->previous != NULL);
  VERIFY(n->next != NULL);

  n->previous->next = n->next;
  n->next->previous = n->previous;

  n->previous = n;
  n->next = n;
}

void append_node_to_list(node *n, node **list) {
  VERIFY(n != NULL && list != NULL);

  if (*list == NULL) {
    *list = n;
  } else {
    node *head = *list;
    VERIFY(head->next != NULL);

    n->previous = head;
    n->next = head->next;

    head->next->previous = n;
    head->next = n;
  }
}