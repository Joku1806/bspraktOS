#include "arch/cpu/mission_control.h"
#include <arch/bsp/pl001.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <lib/debug.h>
#include <arch/bsp/systimer.h>

void start_kernel() {
  for (;;) {
    char ch = pl001_receive();

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
    }
  }

  start_systimer();


  kprintf("Booted!\n");
  //asm volatile("bkpt #0");
  kprintf("After Prefetch Abort!\n");
  for (;;) {}

}
