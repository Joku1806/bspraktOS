#ifndef PROGRAM_STATUS_REGISTER_H
#define PROGRAM_STATUS_REGISTER_H

#define NUMBER_OF_USED_MODES 5

#ifdef __ASSEMBLER__

#define MODE_IRQ 0x12
#define MODE_SUPERVISOR 0x13
#define MODE_ABORT 0x17
#define MODE_UNDEFINED 0x1b
#define MODE_SYSTEM 0x1f

#else

#include <stdint.h>

typedef enum {
  psr_mode = 0x1f,
  psr_T32_instruction_set_state = 0x1 << 5,
  psr_FIQ_interrupt_mask = 0x1 << 6,
  psr_IRQ_interrupt_mask = 0x1 << 7,
  psr_endianness = 0x1 << 9,
  psr_overflow = 0x1 << 28,
  psr_carry = 0x1 << 29,
  psr_zero = 0x1 << 30,
  psr_negative = 0x1 << 31,
} psr_flags;

typedef enum {
  psr_not_initialized = 0x0,
  psr_mode_user = 0x10,
  psr_mode_IRQ = 0x12,
  psr_mode_supervisor = 0x13,
  psr_mode_abort = 0x17,
  psr_mode_undefined = 0x1b,
  psr_mode_system = 0x1f
} psr_mode_bits;

const char *get_mode_name(uint32_t psr);

#endif

#endif