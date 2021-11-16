#include <kernel/kprintf.h>

_Noreturn void halt_cpu() {
  kprintf("System angehalten.\n");
  for (;;) {
    asm volatile("WFI");
  }
}