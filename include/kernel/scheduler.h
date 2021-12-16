#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <arch/cpu/registers.h>
#include <stdint.h>

void schedule_thread(registers *thread_regs);

#endif