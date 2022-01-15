#ifndef SYSTIMER_H
#define SYSTIMER_H

#include <stdbool.h>
#include <stdint.h>

#define SYSTIMER_BASE (0x7E003000 - 0x3F000000)
#define SYSTIMER_FREQUENCY_HZ 1000000

typedef enum {
  CS = 0x0,
  CLO = 0x4,
  C1 = 0x10,
  C3 = 0x18,
} systimer_offsets;

typedef enum {
  M1 = 1 << 1,
  M3 = 1 << 3,
} control_flags;

volatile uint32_t *systimer_register(systimer_offsets offset);

bool systimer_pending();
void systimer_reset();
uint32_t systimer_value();

bool stalltimer_pending();
void stalltimer_reset_pending_interrupt();
int stalltimer_interrupt_at(uint32_t hz);

#endif