#ifndef THREAD_H
#define THREAD_H

#include <lib/bounded_linked_list.h>
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

typedef struct {
  node node;
  uint32_t regs[16];
  uint32_t cpsr;
  size_t index;
} tcb;

extern node *ready_head;
extern node *waiting_head;
extern node *running_head;
extern node *finished_head;

tcb *get_idle_thread();

void reset_thread_context(size_t index);
void save_thread_context(tcb *thread, uint32_t *regs, uint32_t cpsr);
void load_thread_context(tcb *thread, uint32_t *current_thread_regs);
void thread_list_initialise();
void thread_create(void (*func)(void *), const void *args,
                   unsigned int args_size);
void thread_cleanup();

#endif