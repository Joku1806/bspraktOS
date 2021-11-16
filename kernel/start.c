#include <arch/bsp/pl001.h>
#include <config.h>
#include <kernel/kprintf.h>

void start_kernel() {
  kprintf("Booted!\n");
  asm volatile("bkpt #0");
  kprintf("After Prefetch Abort!\n");
  for (;;) {}
}
