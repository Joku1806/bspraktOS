#include <arch/bsp/systimer.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <stdbool.h>

extern bool timer_interrupt_output;

volatile uint32_t *systimer_register(systimer_offsets offset) {
  return (volatile uint32_t *)(SYSTIMER_BASE + offset);
}

void systimer_reset() {
  if (timer_interrupt_output) {
    kprintf("!\n");
  }

  *systimer_register(CS) |= M1;
  *systimer_register(C1) = *systimer_register(CLO) + TIMER_INTERVAL;
}
