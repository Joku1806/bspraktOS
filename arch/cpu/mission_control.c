#include <arch/cpu/mission_control.h>

_Noreturn void halt_cpu() {
  for (;;) {
    asm volatile("WFI");
  }
}