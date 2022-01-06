#ifndef INTRUSIVE_LIST_H
#define INTRUSIVE_LIST_H

#include <stdbool.h>

typedef struct node {
  struct node *previous;
  struct node *next;
} node;

bool is_list_empty(node *list);
bool is_first_node(node *list, node *n);
bool is_last_node(node *list, node *n);
node *get_first_node(node *list);
node *get_last_node(node *list);
void remove_node_from_list(node *list, node *n);
void append_node_to_list(node *list, node *n);
void transfer_list_node(node *from, node *to, node *n);

#endif