#ifndef BOUNDED_THREAD_LIST_H
#define BOUNDED_THREAD_LIST_H

#include <kernel/thread.h>
#include <stdint.h>

#define TCB_LIST_SIZE 32

typedef struct node {
  struct node *previous;
  struct node *next;
} node;

typedef struct {
  node *node;
  uint32_t regs[16];
  uint32_t cpsr;
} tcb;

void thread_list_initialise();
node *get_thread_list_head(thread_status status);
void transfer_node_to_list(node *tcb_node, node *list);

#endif