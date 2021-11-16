#include <arch/cpu/mission_control.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <lib/assertions.h>
#include <lib/string.h>
#include <stdint.h>

void reset_interrupt_handler() {
  kprintf("Willkommen im Reset Interrupt Handler!\n");
  // FIXME: sollte System neustarten, was wir gerade machen
  // ist eine Endlosschleife :D
  halt_cpu();
}

void undefined_instruction_interrupt_handler() {
  kprintf("Willkommen im Undefined Instruction Interrupt Handler!\n");
}

void software_interrupt_handler() {
  kprintf("Willkommen im Software Interrupt Handler!\n");
}

void prefetch_abort_interrupt_handler() {
  kprintf("Willkommen im Prefetch Abort Interrupt Handler!\n");
  uint32_t regs[16] = {0x00000001, 0x00000001, 0x00009cd0, 0xfffffff9,
                       0x07f91450, 0x00000000, 0x00008000, 0x00000000,
                       0x00000000, 0x07b41ee8, 0x07b419a4, 0x07f46a84,
                       0x00000010, 0x000d0000, 0x000080e8, 0x000082a8};
  uint32_t CPSR = 0x600000D7;
  uint32_t SPSR = 0x80000053;
  dump_registers(regs, CPSR, SPSR);
  halt_cpu();
}

void data_abort_interrupt_handler() {
  kprintf("Willkommen im Data Abort Interrupt Handler!\n");
}

void irq_interrupt_handler() {
  kprintf("Willkommen im IRQ Interrupt Handler!\n");
}

void print_register_using_layout(uint32_t reg, register_group *layout_parts,
                                 int part_count) {
  register_group *current = layout_parts;
  for (size_t bit_offset = 0; bit_offset < 32; bit_offset++) {
    while (!current->printable || current->last_member_index < bit_offset) {
      if (current->printable) {
        kprintf(" ");
      }

      bit_offset = current->last_member_index + 1;
      VERIFY(bit_offset <= 31);
      current++;
    }

    if (reg & (BIT_NEEDLE >> bit_offset)) {
      kprintf(
          "%c",
          current
              ->flag_mnemonics[strlen(current->flag_mnemonics) -
                               (current->last_member_index - bit_offset) - 1]);
    } else {
      kprintf("_");
    }
  }

  VERIFY(current - layout_parts == part_count - 1);
}

// TODO: braucht Informationen über aktuellen
// Modus, um spezifische Register auszugeben
void dump_registers(uint32_t general_regs[16], uint32_t CPSR, uint32_t SPSR) {
  kprintf("#############################################"
          "#######################"
          "#######\n");

  kprintf(">>> Registerschnappschuss (aktueller Modus) <<<\n");
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    // FIXME: Unterstütze %#x
    kprintf("R%u: %#010x   ", reg_idx, general_regs[reg_idx]);

    // FIXME: Fälle vereinfachen
    if (reg_idx + 8 < 10) {
      kprintf("R%u:  %#010x\n", reg_idx + 8, general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 13) {
      kprintf("SP:  %#010x\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 14) {
      kprintf("LR:  %#010x\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 15) {
      kprintf("PC:  %#010x\n", general_regs[reg_idx + 8]);
    } else if (reg_idx + 8 != 16) {
      kprintf("R%u: %#010x\n", reg_idx + 8, general_regs[reg_idx + 8]);
    }
  }

  kprintf("\n>>> Aktuelle Statusregister (SPSR des aktuellen Modus) <<<\n");
  register_group groups[5] = {{"NZCV", 3, true},
                              {NULL, 26, false},
                              {"E", 27, true},
                              {NULL, 28, false},
                              {"IFT", 31, true}};
  kprintf("CPSR: ");
  print_register_using_layout(CPSR, groups, 5);
  kprintf("(%032b)\n", CPSR);

  kprintf("SPSR: ");
  print_register_using_layout(SPSR, groups, 5);
  kprintf("(%032b)\n", SPSR);

  kprintf(">>> Aktuelle modusspezifische Register <<<\n");

  halt_cpu();
}