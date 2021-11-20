#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <arch/cpu/cpu_modes.h>
#include <stddef.h>
#include <stdint.h>

#define BIT_NEEDLE 0x80000000
#define REGISTER_BIT_WIDTH 32

#define LR_POSITION 14

#define GET_MODE_REGS(mode, store)                                             \
  asm volatile("mrs r0, cpsr \n\t"                                             \
               "cps %3 \n\t"                                                   \
               "mrs %0, spsr \n\t"                                             \
               "mov %1, lr \n\t"                                               \
               "mov %2, sp \n\t"                                               \
               "msr cpsr_cxsf, r0 \n\t"                                        \
               : "=r"(*(store)), "=r"(*((store) + 1)), "=r"(*((store) + 2))    \
               : "I"(mode)                                                     \
               : "r0", "memory")

typedef struct {
  char *mnemonic;
  size_t last_member_bit_offset;
} register_layout_part;

void reset_interrupt_handler(uint32_t *regs);
void undefined_instruction_interrupt_handler(uint32_t *regs);
void software_interrupt_handler(uint32_t *regs);
void prefetch_abort_interrupt_handler(uint32_t *regs);
void data_abort_interrupt_handler(uint32_t *regs);
void irq_interrupt_handler(uint32_t *regs);

#endif