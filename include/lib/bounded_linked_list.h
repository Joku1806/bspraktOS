#ifndef BOUNDED_LINKED_LIST_H
#define BOUNDED_LINKED_LIST_H

typedef struct node {
  struct node *previous;
  struct node *next;
} node;

void remove_node_from_current_list(node *n);
void append_node_to_list(node *n, node **list);

#endif