#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <kernel/lib/kintrusive_list.h>
#include <kernel/mmu.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define USER_THREAD_COUNT 32
#define THREAD_COUNT USER_THREAD_COUNT + 1
#define IDLE_THREAD_INDEX USER_THREAD_COUNT

#define ADDRESS_SPACE_COUNT 8
#define THREADS_PER_ADDRESS_SPACE 4

#define CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY true
#if CONSTANTLY_VERIFY_THREAD_LIST_INTEGRITY
void scheduler_verify_thread_list_integrity();
#endif

typedef struct {
  k_node scheduler_node;
  k_node addrspace_node;
  registers regs;
  uint32_t cpsr;
  size_t pid;
  size_t pid_slot;
  size_t tid;
  l2_handle stack_handle;
  unsigned stall_until;
} tcb;

typedef enum {
  normal,
  forced,
} scheduling_type;

void scheduler_initialise();
void scheduler_setup_stack_mappings(l2_entry *stack_tables);
bool scheduler_find_first_empty_address_space(size_t *index);
bool scheduler_find_first_empty_slot_in_address_space(size_t address_space, size_t *index);
const char *scheduler_get_list_name(k_node *head);
tcb *scheduler_get_running_thread();
tcb *scheduler_get_idle_thread();

void scheduler_create_process(size_t address_space, void (*func)(void *), const void *args, unsigned int args_size);
void scheduler_create_thread(tcb *caller, size_t process_slot, void (*func)(void *), const void *args, unsigned int args_size);
void scheduler_save_thread_context(tcb *thread, registers *regs);
void scheduler_switch_thread(registers *current_thread_regs, tcb *thread);
void scheduler_stall_thread(unsigned ms);
void scheduler_cleanup_thread();

void scheduler_ignore_thread_until_character_input(tcb *thread);
void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match);

bool scheduler_exists_input_waiting_thread();
void scheduler_unblock_first_input_waiting_thread(char ch);
void scheduler_unblock_overdue_waiting_threads();
int scheduler_adjust_stall_timer();

void scheduler_round_robin(registers *thread_regs, scheduling_type type);

#endif