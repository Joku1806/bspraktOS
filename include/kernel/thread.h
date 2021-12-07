#ifndef THREAD_H
#define THREAD_H

#include <stddef.h>
#include <stdint.h>

#define TCB_LIST_SIZE 32

typedef enum {
  ready = 0,
  waiting = 1,
  running = 2,
  finished = 3,
} thread_status;

typedef struct node {
  struct node *previous;
  struct node *next;
} node;

typedef struct {
  node *node;
  uint32_t regs[16];
  uint32_t cpsr;
  size_t index;
} tcb;

void reset_thread_context(size_t index);
void thread_list_initialise();
node *get_thread_list_head(thread_status status);
void transfer_thread_block_to_list(node *tcb_node, node *list);
void thread_cleanup();

#endif