#ifndef KINTRUSIVE_LIST_H
#define KINTRUSIVE_LIST_H

#include <stdbool.h>

// definiert in https://github.com/torvalds/linux/blob/bf4eebf8cfa2cd50e20b7321dfb3effdcdc6e909/tools/include/linux/kernel.h
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })
#endif

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