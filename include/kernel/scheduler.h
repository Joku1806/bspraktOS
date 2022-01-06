#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <kernel/thread.h>
#include <stdbool.h>
#include <stdint.h>

bool is_thread_available();

void scheduler_ignore_thread_until_character_input(tcb *thread);
void scheduler_ignore_thread_until_timer_match(tcb *thread, unsigned match);

void scheduler_unblock_input_waiting_threads(char ch);
void scheduler_unblock_stall_waiting_threads(unsigned current_time);
void schedule_thread(registers *thread_regs);

#endif