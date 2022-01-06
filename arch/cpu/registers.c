#include <arch/cpu/registers.h>
#include <stdint.h>

uint32_t get_current_sp() {
  uint32_t sp;
  asm("mov %0, sp \n\t"
      : "=r"(sp));
  return sp;
}

void set_current_sp(uint32_t sp) {
  asm("mov sp, %0 \n\t"
      : "=r"(sp));
}