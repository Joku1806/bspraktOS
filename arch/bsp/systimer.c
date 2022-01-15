#include "kernel/lib/kdebug.h"
#include <arch/bsp/systimer.h>
#include <config.h>
#include <kernel/lib/kerror.h>
#include <kernel/lib/kmath.h>
#include <stdbool.h>
#include <stdint.h>

volatile uint32_t *systimer_register(systimer_offsets offset) {
  return (volatile uint32_t *)(SYSTIMER_BASE + offset);
}

void systimer_reset() {
  *systimer_register(CS) |= M1;
  *systimer_register(C1) = *systimer_register(CLO) + TIMER_INTERVAL;
}

uint32_t systimer_value() { return *systimer_register(CLO); }

void stalltimer_reset_pending_interrupt() {
  *systimer_register(CS) |= M3;
}

#define TOCTOU_PAD 20

int stalltimer_interrupt_at(uint32_t hz) {
  *systimer_register(CS) |= M3;

  if (systimer_value() + TOCTOU_PAD < hz) {
    *systimer_register(C3) = hz;
    kdbgln("Set C3 = %#010x", hz);
    return 0;
  }

  return -K_EINVAL;
}
