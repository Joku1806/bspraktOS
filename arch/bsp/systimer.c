#include <config.h>
#include <kernel/kprintf.h>
#include <lib/debug.h>
#include <arch/bsp/systimer.h>

volatile uint32_t *load_register(systimer_offsets offset) {
  return (volatile uint32_t *)(UART_TIMER + offset);
}


void start_systimer(){
  *load_register(CS) &=~ M1; 
  *load_register(C1) += TIMER_INTERVAL;
}

void reset_timer(){
  *load_register(CS) |= M1;
}
