#include <arch/cpu/CPU.h>
#include <kernel/kprintf.h>
#include <stddef.h>
#include <stdint.h>

// TODO: braucht Informationen über aktuellen
// Modus, um spezifische Register auszugeben
void dump_registers(uint32_t general_regs[16], uint32_t SPSR, uint32_t CPSR) {
  kprintf("#############################################"
          "#######################"
          "#######\n");

  kprintf(">>> Registerschnappschuss (aktueller Modus) <<<\n");
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    // FIXME: Unterstütze %#x
    kprintf("R%u: %p   ", reg_idx, general_regs[reg_idx]);

    if (reg_idx + 8 < 10) {
      kprintf("R%u:  %p\n", reg_idx + 8, general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 13) {
      kprintf("SP:  %p\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 14) {
      kprintf("LR:  %p\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 15) {
      kprintf("PC:  %p\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 != 16) {
      kprintf("R%u: %p\n", reg_idx + 8, general_regs[reg_idx + 8]);
    }
  }

  kprintf("\n>>> Aktuelle Statusregister (SPSR des aktuellen Modus) <<<\n");
  kprintf(">>> Aktuelle modusspezifische Register <<<\n");

  halt_cpu();
}

_Noreturn void halt_cpu() {
  kprintf("System angehalten.\n");
  for (;;) {
    asm volatile("WFI");
  }
}