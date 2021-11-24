#include <arch/bsp/systimer.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <lib/debug.h>

volatile uint32_t *systimer_register(systimer_offsets offset) {
  return (volatile uint32_t *)(TIMER_BASE + offset);
}

void reset_systimer() {
  *systimer_register(CS) |= M1;
  *systimer_register(C1) = *systimer_register(CLO) + TIMER_INTERVAL;
}
