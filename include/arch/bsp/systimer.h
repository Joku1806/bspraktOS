#include <stdint.h>

#define SYSTIMER_BASE (0x7E003000 - 0x3F000000)

typedef enum {
  CS = 0x0,
  CLO = 0x4,
  C1 = 0x10,
} systimer_offsets;

typedef enum {
  M1 = 1 << 1,
} control_flags;

void systimer_reset();