#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Kernel Start"

#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <kernel/thread.h>
#include <lib/debug.h>
#include <lib/timing.h>
#include <stdbool.h>
#include <stddef.h>

bool print_registers = false;
bool timer_interrupt_output = false;
#define NUM_CALCULATION_CYCLES 50

void important_calculations() {
  timer_interrupt_output = true;
  for (;;) {
    while (!pl001_new_character_arrived()) {}
    for (size_t cycles = 0; cycles < NUM_CALCULATION_CYCLES; cycles++) {
      sleep_macgyver(BUSY_WAIT_COUNTER);
      kprintf("%c", pl001_read());
    }
  }
}

void print_menu() {
  kprintf("Willkommen in unserem Betriebssystem!\n"
          "Interrupts ==========================\n"
          "a: (Thread) Data Abort auslösen\n"
          "p: (Thread) Prefetch Abort auslösen\n"
          "s: (Thread) Software Interrupt auslösen\n"
          "u: (Thread) Undefined Instruction auslösen\n"
          "A: (System) Data Abort auslösen\n"
          "P: (System) Prefetch Abort auslösen\n"
          "S: (System) Software Interrupt auslösen\n"
          "U: (System) Undefined Instruction auslösen\n"
          "Debughilfen =========================\n"
          "c: (Thread) Registerchecker ausführen\n");
}

void start_kernel() {
  // FIXME: Kernel Thread?
  systimer_reset();
  pl001_setup();
  thread_list_initialise();
  print_menu();
}