#ifndef REGISTER_DUMP
#define REGISTER_DUMP

#include <arch/cpu/registers.h>
#include <stddef.h>
#include <stdint.h>

#define BIT_NEEDLE 0x80000000
#define REGISTER_BIT_WIDTH 32
#define MAX_MODE_NAME_LENGTH 10

#define GET_MODE_REGS(mode, store)                                          \
  asm volatile("mrs r0, cpsr \n\t"                                          \
               "cps %3 \n\t"                                                \
               "mrs %0, spsr \n\t"                                          \
               "mov %1, lr \n\t"                                            \
               "mov %2, sp \n\t"                                            \
               "msr cpsr_cxsf, r0 \n\t"                                     \
               : "=r"(*(store)), "=r"(*((store) + 1)), "=r"(*((store) + 2)) \
               : "I"(mode)                                                  \
               : "memory")

typedef struct {
  char *mnemonic;
  size_t last_member_bit_offset;
} register_layout_part;

typedef enum {
  reset,
  undefined_instruction,
  software,
  prefetch_abort,
  data_abort,
  irq,
} interrupt_type;

void print_register_using_layout(uint32_t reg, register_layout_part *layout);
void print_current_mode_status_registers(register_layout_part *layout);
void print_general_registers(registers *regs);
void print_various_mode_registers(register_layout_part *layout);
void dump_registers(interrupt_type type, registers *regs);

#endif