#include <arch/cpu/psr.h>
#include <lib/assertions.h>
#include <stdint.h>

const char *get_mode_name(uint32_t psr) {
  switch (psr & psr_mode) {
    case psr_mode_user:
      return "User";
    case psr_mode_IRQ:
      return "IRQ";
    case psr_mode_supervisor:
      return "Supervisor";
    case psr_mode_abort:
      return "Abort";
    case psr_mode_undefined:
      return "Undefined";
    case psr_mode_system:
      return "System";
    case psr_not_initialized:
      return "Not Initialized";
  }

  VERIFY_NOT_REACHED();
}

uint32_t get_spsr() {
  uint32_t spsr;
  asm("msr %0, spsr \t\n" : "=r"(spsr));
  return spsr;
}