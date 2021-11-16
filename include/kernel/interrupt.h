#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdbool.h>
#include <stddef.h>

void reset_interrupt_handler();
void undefined_instruction_interrupt_handler();
void software_interrupt_handler();
void prefetch_abort_interrupt_handler();
void data_abort_interrupt_handler();
void irq_interrupt_handler();

#define BIT_NEEDLE 0x80000000

// FIXME: besseren Namen finden
typedef struct {
  char *flag_mnemonics;
  size_t last_member_index;
  bool printable;
} register_group;

void dump_registers();

#endif