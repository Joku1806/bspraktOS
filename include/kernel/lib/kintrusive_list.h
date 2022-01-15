#ifndef KINTRUSIVE_LIST_H
#define KINTRUSIVE_LIST_H

#include <stdbool.h>

typedef struct k_node {
  struct k_node *previous;
  struct k_node *next;
} k_node;

// FIXME: Funktionsnamen müssen konsistenter sein.
bool k_is_list_empty(k_node *list);
bool k_is_list_node(k_node *n);
bool k_is_first_node(k_node *list, k_node *n);
bool k_is_last_node(k_node *list, k_node *n);
k_node *k_get_first_node(k_node *list);
k_node *k_get_last_node(k_node *list);
void k_remove_node_from_list(k_node *list, k_node *n);
void k_append_node_to_list(k_node *list, k_node *n);
void k_insert_sorted(k_node *list, k_node *n, bool (*cmp)(k_node *, k_node *));

void k_transfer_list_node(k_node *from, k_node *to, k_node *n);

#endif