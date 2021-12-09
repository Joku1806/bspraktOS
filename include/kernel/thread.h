#ifndef THREAD_H
#define THREAD_H

#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 32

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
  node node;
  uint32_t regs[16];
  uint32_t cpsr;
  size_t index;
} tcb;

node *ready_head;
node *waiting_head;
node *running_head;
node *finished_head;

void reset_thread_context(size_t index);
void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr);
void load_thread_context(tcb *thread);
void thread_list_initialise();
void transfer_thread_block_to_list(node *tcb_node, node *list);
void thread_cleanup();

#endif