#ifndef INTERRUPT_H
#define INTERRUPT_H

void reset_interrupt_handler();
void undefined_instruction_interrupt_handler();
void software_interrupt_handler();
void prefetch_abort_interrupt_handler();
void data_abort_interrupt_handler();
void irq_interrupt_handler();

void dump_registers();

#endif