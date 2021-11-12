#include <kernel/kprintf.h>

// TODO: braucht Informationen über aktuellen
// Modus, um spezifische Register auszugeben
void dump_registers() {}

_Noreturn void halt_cpu() {
  kprintf("System angehalten.\n");
  for (;;) {
    asm volatile("WFI");
  }
}