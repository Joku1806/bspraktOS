#include <arch/cpu/mission_control.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <lib/assertions.h>
#include <lib/string.h>
#include <stdint.h>

void reset_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im Reset Interrupt Handler!\n");
  // FIXME: sollte System neustarten, was wir gerade machen
  // ist eine Endlosschleife :D
  halt_cpu();
}

void undefined_instruction_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im Undefined Instruction Interrupt Handler!\n");
}

void software_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im Software Interrupt Handler!\n");
}

void prefetch_abort_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im Prefetch Abort Interrupt Handler!\n");
  dump_registers(regs, prefetch_abort);
}

void data_abort_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im Data Abort Interrupt Handler!\n");
}

void irq_interrupt_handler(interrupt_registers *regs) {
  kprintf("Willkommen im IRQ Interrupt Handler!\n");
}

void print_register_using_layout(uint32_t reg,
                                 register_layout_part *layout_parts) {
  register_layout_part *part = layout_parts;
  for (size_t bit_offset = 0; bit_offset < REGISTER_BIT_WIDTH; bit_offset++) {
    if (bit_offset > part->last_member_bit_offset) {
      kprintf(" ");
      part++;
    }

    if (!part->printable) {
      bit_offset = part->last_member_bit_offset;
      part++;
      continue;
    }

    VERIFY(bit_offset < REGISTER_BIT_WIDTH);
    if (reg & (BIT_NEEDLE >> bit_offset)) {
      size_t flag_index = strlen(part->mnemonic) -
                          (part->last_member_bit_offset - bit_offset) - 1;
      VERIFY(flag_index < strlen(part->mnemonic));
      kprintf("%c", part->mnemonic[flag_index]);
    } else {
      kprintf("_");
    }
  }
}

// TODO: braucht Informationen über aktuellen
// Modus, um spezifische Register auszugeben
void dump_registers(interrupt_registers *regs, interrupt_mode mode) {
  kprintf("#############################################"
          "#######################"
          "#######\n");

  kprintf(">>> Registerschnappschuss (aktueller Modus) <<<\n");
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    // FIXME: Unterstütze %#x
    kprintf("R%u: %#010x   ", reg_idx, regs->general[reg_idx]);

    // FIXME: Fälle vereinfachen
    if (reg_idx + 8 < 10) {
      kprintf("R%u:  %#010x\n", reg_idx + 8, regs->general[reg_idx + 8]);
    } else if (reg_idx + 8 == 13) {
      kprintf("SP:  %#010x\n", regs->general[reg_idx + 8]);
    } else if (reg_idx + 8 == 14) {
      kprintf("LR:  %#010x\n", regs->general[reg_idx + 8]);
    } else if (reg_idx + 8 == 15) {
      kprintf("PC:  %#010x\n", regs->general[reg_idx + 8]);
    } else if (reg_idx + 8 != 16) {
      kprintf("R%u: %#010x\n", reg_idx + 8, regs->general[reg_idx + 8]);
    }
  }

  kprintf("\n>>> Aktuelle Statusregister (SPSR des aktuellen Modus) <<<\n");
  register_layout_part groups[5] = {
      {.mnemonic = "NZCV", .last_member_bit_offset = 3, .printable = true},
      {.mnemonic = NULL, .last_member_bit_offset = 26, .printable = false},
      {.mnemonic = "E", .last_member_bit_offset = 27, .printable = true},
      {.mnemonic = NULL, .last_member_bit_offset = 28, .printable = false},
      {.mnemonic = "IFT", .last_member_bit_offset = 31, .printable = true}};

  kprintf("CPSR: ");
  print_register_using_layout(regs->cpsr, groups);
  kprintf("(%#010x)\n", regs->cpsr);

  kprintf("SPSR: ");
  print_register_using_layout(regs->spsr, groups);
  kprintf("(%#010x)\n", regs->spsr);

  kprintf(">>> Aktuelle modusspezifische Register <<<\n");
  halt_cpu();
}