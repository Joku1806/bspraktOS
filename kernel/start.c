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
      // FIXME: BUSY_WAIT_COUNTER sollte einfach in einer while-Schleife
      // hochzählen, kein Wunder dass das so verdammt langsam ist
      sleep_mhz(BUSY_WAIT_COUNTER);
      kprintf("%c", pl001_read());
    }
  }
}

void print_menu() {
  kprintf("Willkommen in unserem Betriebssystem!\n"
          "Interrupts ==========================\n"
          "a: Data Abort auslösen\n"
          "p: Prefetch Abort auslösen\n"
          "s: Software Interrupt auslösen\n"
          "u: Undefined Instruction auslösen\n"
          "Debughilfen =========================\n"
          "c: Registerchecker ausführen\n"
          "d: Registerdump an/ausschalten\n"
          "Funktionalität ======================\n"
          "e: Unterprogramm ausführen\n"
          "?: Dieses Menü anzeigen\n\n");
}

void start_kernel() {
  // FIXME: Sollten diese drei Sachen schon vorher als Setup gemacht werden?
  systimer_reset();
  pl001_setup();
  thread_list_initialise();
  print_menu();

  for (;;) {
    while (!pl001_new_character_arrived()) {}
    char ch = pl001_read();
    switch (ch) {
      case 'a':
        asm volatile("mov r0, #0x1 \n ldr r0, [r0]");
        break;
      case 'p':
        asm volatile("bkpt #0");
        break;
      case 's':
        asm volatile("svc #0");
        break;
      case 'u':
        asm volatile(".word 0xf7f0a000\n");
        break;
      case 'c':
        register_checker();
        break;
      case 'd':
        print_registers = !print_registers;
        break;
      case 'e':
        important_calculations();
        break;
      case '?':
        print_menu();
        break;
    }
  }
}
