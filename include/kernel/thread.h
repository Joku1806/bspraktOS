#ifndef THREAD_H
#define THREAD_H

#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 32
#define IDLE_THREAD_INDEX USER_THREAD_COUNT

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

tcb idle_thread;
node *ready_head;
node *waiting_head;
node *running_head;
node *finished_head;

void reset_thread_context(size_t index);
void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr);
void load_thread_context(tcb *thread, uint32_t *current_thread_regs);
void thread_list_initialise();
// FIXME: Sollten die in einer extra Funktion kombiniert werden? Im Moment
// kommen sie nur zusammen vor. Außerdem sollte thread_block -> node umbenannt
// werden und diese Funktionalität in lib abgekapselt werden.
void remove_node_from_current_list(node *thread);
void append_node_to_list(node *thread, node **list);
void thread_cleanup();

#endif