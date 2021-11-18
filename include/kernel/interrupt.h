#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <arch/cpu/cpu_modes.h>
#include <stddef.h>
#include <stdint.h>

#define BIT_NEEDLE 0x80000000
#define REGISTER_BIT_WIDTH 32

typedef struct {
  char *mnemonic;
  size_t last_member_bit_offset;
} register_layout_part;

typedef struct {
  uint32_t cpsr;
  uint32_t spsr;
  uint32_t general[16];
} interrupt_registers;

void reset_interrupt_handler(interrupt_registers *regs);
void undefined_instruction_interrupt_handler(interrupt_registers *regs);
void software_interrupt_handler(interrupt_registers *regs);
void prefetch_abort_interrupt_handler(interrupt_registers *regs);
void data_abort_interrupt_handler(interrupt_registers *regs);
void irq_interrupt_handler(interrupt_registers *regs);

void dump_registers(interrupt_registers *regs);

// FIXME: Header-Datei?
extern uint32_t get_mode_spsr(cpu_mode mode);
extern uint32_t get_mode_lr(cpu_mode mode);
extern uint32_t get_mode_pc(cpu_mode mode);

#endif