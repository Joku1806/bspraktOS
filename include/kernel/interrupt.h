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
#define REGISTER_BIT_WIDTH 32

typedef struct {
  char *mnemonic;
  size_t last_member_bit_offset;
  bool printable;
} register_layout_part;

void dump_registers();

#endif