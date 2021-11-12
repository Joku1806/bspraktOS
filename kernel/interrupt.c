#include <kernel/kprintf.h>

void reset_interrupt_handler() {
  kprintf("Willkommen im Reset Interrupt Handler!\n");
}

void undefined_instruction_interrupt_handler() {
  kprintf("Willkommen im Undefined Instruction Interrupt Handler!\n");
}

void software_interrupt_handler() {
  kprintf("Willkommen im Software Interrupt Handler!\n");
}

void prefetch_abort_interrupt_handler() {
  kprintf("Willkommen im Prefetch Abort Interrupt Handler!\n");
}

void data_abort_interrupt_handler() {
  kprintf("Willkommen im Data Abort Interrupt Handler!\n");
}

void irq_interrupt_handler() {
  kprintf("Willkommen im IRQ Interrupt Handler!\n");
}