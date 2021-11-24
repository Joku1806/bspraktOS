#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <lib/debug.h>
#include <stdbool.h>

bool print_registers = true;

void start_kernel() {
  reset_systimer();
  pl001_setup();
  
  for (;;) {
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
    }
  }
}
