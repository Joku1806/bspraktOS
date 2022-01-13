#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Intrusive List"

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kintrusive_list.h>
#include <stdbool.h>
#include <stddef.h>

bool k_is_list_head(k_node *n) {
  VERIFY(n != NULL);
  return n->previous == NULL;
}

bool k_is_list_node(k_node *n) {
  VERIFY(n != NULL);
  return n->previous != NULL && n->next != NULL;
}

bool k_is_list_empty(k_node *list) {
  VERIFY(k_is_list_head(list));
  return list->next == NULL;
}

bool k_is_first_node(k_node *list, k_node *n) {
  if (k_is_list_empty(list)) {
    return false;
  }

  return k_get_first_node(list) == n;
}

bool k_is_last_node(k_node *list, k_node *n) {
  if (k_is_list_empty(list)) {
    return false;
  }

  return k_get_last_node(list) == n;
}

k_node *k_get_first_node(k_node *list) {
  VERIFY(k_is_list_head(list));
  VERIFY(!k_is_list_empty(list));
  VERIFY(k_is_list_node(list->next));
  return list->next;
}

k_node *k_get_last_node(k_node *list) {
  VERIFY(k_is_list_head(list));
  VERIFY(!k_is_list_empty(list));
  VERIFY(k_is_list_node(list->next->previous));
  return list->next->previous;
}

void k_connect_nodes(k_node *from, k_node *to) {
  VERIFY(k_is_list_node(from));
  VERIFY(k_is_list_node(to));
  kdbgln("Connecting node %p to node %p.", from, to);

  from->next = to;
  to->previous = from;
}

void k_remove_node_from_list(k_node *list, k_node *n) {
  VERIFY(k_is_list_head(list));
  VERIFY(k_is_list_node(n));
  kdbgln("Removing node %p from list %p.", n, p);

  if (list->next == n) {
    kdbgln("Node %p is first node in list.", n);
    // FIXME: wird der erste Check überhaupt gebraucht?
    if (n == n->previous && n == n->next) {
      kdbgln("Node %p is also only node in list, clearing list head.", n);
      list->next = NULL;
    } else {
      kdbgln("Pointing head of list %p to second node %p.", list, n->next);
      list->next = n->next;
    }
  }

  k_connect_nodes(n->previous, n->next);
  k_connect_nodes(n, n);
}

// FIXME: list muss nicht unbedingt ein list head sein
void k_append_node_to_list(k_node *list, k_node *n) {
  VERIFY(k_is_list_node(list) || k_is_list_head(list));
  VERIFY(k_is_list_node(n));
  VERIFY(list != n);

  if (k_is_list_head(list)) {
    if (list->next == NULL) {
      list->next = n;
      return;
    }

    // am Ende der Liste anhängen ist das gleiche wie als erstes Element in die Liste einfügen
    k_node *head = list;
    list = k_get_last_node(list);
    kdbgln("Pointing head of list %p to node %p.", head, n);
    head->next = n;
  }

  k_connect_nodes(n, list->next);
  k_connect_nodes(list, n);
}

void k_transfer_list_node(k_node *from, k_node *to, k_node *n) {
  k_remove_node_from_list(from, n);
  k_append_node_to_list(to, n);
}