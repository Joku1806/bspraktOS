#include <arch/bsp/kprintf.h>
#include <arch/bsp/yellow_led.h>

volatile unsigned int counter = 0;

void increment_counter() { counter++; }

void start_kernel() {

  yellow_on();
  kprintf("Hello from kernel land! %c", 'A');

  // Endless counter
  for (;;) {
    increment_counter();
  }
}
