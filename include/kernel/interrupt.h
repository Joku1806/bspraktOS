#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <arch/cpu/registers.h>

void reset_interrupt_handler(registers *regs);
void undefined_instruction_interrupt_handler(registers *regs);
void software_interrupt_handler(registers *regs);
void prefetch_abort_interrupt_handler(registers *regs);
void data_abort_interrupt_handler(registers *regs);
void irq_interrupt_handler(registers *regs);

#endif