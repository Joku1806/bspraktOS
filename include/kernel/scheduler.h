#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <lib/intrusive_list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 32
#define THREAD_COUNT USER_THREAD_COUNT + 1
#define IDLE_THREAD_INDEX USER_THREAD_COUNT

#define CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY true
#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
void scheduler_verify_thread_list_integrity();
#define transfer_list_node(from, to, n)       \
  do {                                        \
    transfer_list_node(from, to, n);          \
    scheduler_verify_thread_list_integrity(); \
  } while (0)
#endif

typedef enum {
  ready = 0,
  stall_waiting = 1,
  input_waiting = 2,
  running = 3,
  finished = 4,
} thread_status;

typedef struct {
  node node;
  registers regs;
  uint32_t cpsr;
  size_t id;
  unsigned stall_until;
} tcb;

node *scheduler_get_idle_thread();
node *get_thread_list_head(thread_status status);

const char *get_list_name(node *head);
size_t get_thread_id(node *n);

void reset_thread_context(size_t index);
void save_thread_context(tcb *thread, registers *regs);
void perform_stack_context_switch(registers *current_thread_regs, tcb *thread);
void thread_list_initialise();
void thread_create(void (*func)(void *), const void *args, unsigned int args_size);
void thread_stall(unsigned ms);
void thread_cleanup();

bool scheduler_is_thread_available();
tcb *scheduler_get_running_thread();

void scheduler_ignore_thread_until_character_input(tcb *thread);
void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match);

void scheduler_unblock_input_waiting_threads(char ch);
void scheduler_unblock_stall_waiting_threads(unsigned current_time);
void schedule_thread(registers *thread_regs);

#endif