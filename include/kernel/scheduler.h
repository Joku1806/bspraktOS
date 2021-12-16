#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <stdbool.h>
#include <stdint.h>

bool is_thread_available();
void schedule_thread(registers *thread_regs);

#endif