#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <kernel/lib/kintrusive_list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 32
#define THREAD_COUNT USER_THREAD_COUNT + 1
#define IDLE_THREAD_INDEX USER_THREAD_COUNT

#define CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY true
#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
void scheduler_verify_thread_list_integrity();
#endif

typedef struct {
  k_node node;
  registers regs;
  uint32_t cpsr;
  size_t id;
  unsigned stall_until;
} tcb;

void scheduler_initialise();
const char *scheduler_get_list_name(k_node *head);
size_t scheduler_get_thread_id(k_node *n);
tcb *scheduler_get_running_thread();
k_node *scheduler_get_idle_thread();
bool scheduler_is_thread_available();

void scheduler_reset_thread_context(size_t index);
void scheduler_save_thread_context(tcb *thread, registers *regs);
void scheduler_switch_thread(registers *current_thread_regs, tcb *thread);
void scheduler_create_thread(void (*func)(void *), const void *args, unsigned int args_size);
void scheduler_stall_thread(unsigned ms);
void scheduler_cleanup_thread();

void scheduler_ignore_thread_until_character_input(tcb *thread);
void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match);

bool scheduler_exists_input_waiting_thread();
void scheduler_unblock_first_input_waiting_thread(char ch);
void scheduler_unblock_overdue_waiting_threads();
int scheduler_adjust_stall_timer();

void scheduler_round_robin(registers *thread_regs);
void scheduler_forced_round_robin(registers *thread_regs);

#endif