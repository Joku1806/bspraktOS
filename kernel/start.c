#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <lib/debug.h>
#include <stdbool.h>
#include <stddef.h>

bool print_registers = false;
#define NUM_CALCULATION_CYCLES 50

void wait_for_counter(size_t target) {
  size_t counter = 0;

  while (counter != target) {
    asm volatile("" ::: "memory");
    counter++;
  }
}

void important_calculations() {
  for (;;) {
    while (!pl001_new_character_arrived()) {}
    for (size_t cycles = 0; cycles < NUM_CALCULATION_CYCLES; cycles++) {
      wait_for_counter(BUSY_WAIT_COUNTER);
      char ch = pl001_read();
      kprintf("%c", ch);
    }
  }
}

void start_kernel() {
  reset_systimer();
  pl001_setup();

  for (;;) {
    while (!pl001_new_character_arrived()) {}
    char ch = pl001_read();

    switch (ch) {
      case 's':
        asm volatile("svc #0");
        break;
      case 'a':
        asm volatile("mov r0, #0x1 \n ldr r0, [r0]");
        break;
      case 'u':
        asm volatile(".word 0xf7f0a000\n");
        break;
      case 'p':
        asm volatile("bkpt #0");
        break;
      case 'd':
        print_registers = !print_registers;
        break;
      case 'c':
        register_checker();
        break;
      case 'e':
        important_calculations();
        break;
    }
  }
}
