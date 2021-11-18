#include <arch/cpu/mission_control.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <lib/assertions.h>
#include <lib/string.h>
#include <stdint.h>

void reset_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  // FIXME: sollte System neustarten, was wir gerade machen
  // ist eine Endlosschleife :D
  kprintf("Reset Interrupt an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void undefined_instruction_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Undefined Instruction an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void software_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Software Interrupt an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void prefetch_abort_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Prefetch Abort an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void data_abort_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Data Abort an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void irq_interrupt_handler(interrupt_registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("IRQ Interrupt an Adresse %#010x\n", regs->general[14]);
  dump_registers(regs);
  halt_cpu();
}

void print_register_using_layout(uint32_t reg,
                                 register_layout_part *layout_parts) {
  register_layout_part *part = layout_parts;
  for (size_t bit_offset = 0; bit_offset < REGISTER_BIT_WIDTH; bit_offset++) {
    if (bit_offset > part->last_member_bit_offset) {
      kprintf(" ");
      part++;
    }

    if (part->mnemonic == NULL) {
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

const char *get_padded_mode_label(cpu_mode mode) {
  switch (mode) {
    case m_irq:
      return "IRQ:        ";
    case m_supervisor:
      return "Supervisor: ";
    case m_abort:
      return "Abort:      ";
    case m_undefined:
      return "Undefined:  ";
    case m_system:
      return "User/System:";
    default:
      VERIFY_NOT_REACHED();
  }
}

void dump_registers(interrupt_registers *regs) {
  kprintf("\n>>> Registerschnappschuss (aktueller Modus) <<<\n");
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
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
      {.mnemonic = "NZCV", .last_member_bit_offset = 3},
      {.mnemonic = NULL, .last_member_bit_offset = 26},
      {.mnemonic = "E", .last_member_bit_offset = 27},
      {.mnemonic = NULL, .last_member_bit_offset = 28},
      {.mnemonic = "IFT", .last_member_bit_offset = 31}};

  kprintf("CPSR: ");
  print_register_using_layout(regs->cpsr, groups);
  kprintf(" (%#010x)\n", regs->cpsr);

  kprintf("SPSR: ");
  print_register_using_layout(regs->spsr, groups);
  kprintf(" (%#010x)\n", regs->spsr);

  kprintf("\n>>> Aktuelle modusspezifische Register <<<\n");
  static const cpu_mode modes[NUMBER_OF_MODES] = {m_system, m_supervisor,
                                                  m_abort, m_irq, m_undefined};

  kprintf("             LR         SP         SPSR\n");
  for (size_t mode_index = 0; mode_index < NUMBER_OF_MODES; mode_index++) {
    uint32_t m_spsr = get_mode_spsr(modes[mode_index]);
    uint32_t m_lr = get_mode_lr(modes[mode_index]);
    uint32_t m_pc = get_mode_pc(modes[mode_index]);
    kprintf("%s %#010x %#010x ", get_padded_mode_label(modes[mode_index]), m_lr,
            m_pc);
    print_register_using_layout(m_spsr, groups);
    kprintf(" (%#010x)\n", m_spsr);
  }
  kprintf("\n");
}