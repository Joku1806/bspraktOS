#ifndef THREAD_H
#define THREAD_H

#include <arch/cpu/registers.h>
#include <lib/bounded_linked_list.h>
#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 4
#define IDLE_THREAD_INDEX USER_THREAD_COUNT

typedef enum {
  ready = 0,
  waiting = 1,
  running = 2,
  finished = 3,
} thread_status;

typedef struct {
  node node;
  registers regs;
  uint32_t cpsr;
  size_t index;
} tcb;

node *get_idle_thread();
node *get_thread_list_head(thread_status status);

const char *get_list_name(node *head);
size_t get_thread_id(node *n);

void reset_thread_context(size_t index);
void save_thread_context(tcb *thread, registers *regs, uint32_t cpsr);
void perform_stack_context_switch(registers *current_thread_regs, tcb *thread);
void thread_list_initialise();
void thread_create(void (*func)(void *), const void *args,
                   unsigned int args_size);
void thread_cleanup();

#endif