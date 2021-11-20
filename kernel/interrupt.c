#include <arch/cpu/mission_control.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <lib/assertions.h>
#include <lib/string.h>
#include <stdint.h>

const char *get_padded_mode_label(cpu_mode mode);
void print_register_using_layout(uint32_t reg, register_layout_part *layout);

void print_current_mode_status_registers(register_layout_part *layout);
void print_general_registers(uint32_t *regs);
void print_various_mode_registers(register_layout_part *layout);

void dump_registers(uint32_t *regs);

void reset_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  // FIXME: sollte System neustarten, was wir gerade machen
  // ist eine Endlosschleife :D
  kprintf("Reset Interrupt an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
  halt_cpu();
}

void undefined_instruction_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Undefined Instruction an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
  halt_cpu();
}

void software_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Software Interrupt an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
  halt_cpu();
}

void prefetch_abort_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Prefetch Abort an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
  halt_cpu();
}

void data_abort_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Data Abort an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
  halt_cpu();
}

void irq_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("IRQ Interrupt an Adresse %#010x\n", regs[LR_POSITION]);
  dump_registers(regs);
}

void print_general_registers(uint32_t *regs) {
  kprintf("\n>>> Registerschnappschuss (aktueller Modus) <<<\n");
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    kprintf("R%u: %#010x   ", reg_idx, regs[reg_idx]);

    // FIXME: Fälle vereinfachen
    if (reg_idx + 8 < 10) {
      kprintf("R%u:  %#010x\n", reg_idx + 8, regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 13) {
      kprintf("SP:  %#010x\n", regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 14) {
      kprintf("LR:  %#010x\n", regs[reg_idx + 8]);
    } else if (reg_idx + 8 == 15) {
      kprintf("PC:  %#010x\n", regs[reg_idx + 8]);
    } else if (reg_idx + 8 != 16) {
      kprintf("R%u: %#010x\n", reg_idx + 8, regs[reg_idx + 8]);
    }
  }
}

void print_register_using_layout(uint32_t reg, register_layout_part *layout) {
  for (size_t bit_offset = 0; bit_offset < REGISTER_BIT_WIDTH; bit_offset++) {
    if (bit_offset > layout->last_member_bit_offset) {
      kprintf(" ");
      layout++;
    }

    if (layout->mnemonic == NULL) {
      bit_offset = layout->last_member_bit_offset;
      layout++;
      continue;
    }

    VERIFY(bit_offset < REGISTER_BIT_WIDTH);
    if (reg & (BIT_NEEDLE >> bit_offset)) {
      size_t flag_index = strlen(layout->mnemonic) -
                          (layout->last_member_bit_offset - bit_offset) - 1;
      VERIFY(flag_index < strlen(layout->mnemonic));
      kprintf("%c", layout->mnemonic[flag_index]);
    } else {
      kprintf("_");
    }
  }
}

void print_current_mode_status_registers(register_layout_part *layout) {
  uint32_t cpsr = 0, spsr = 0;
  asm volatile("mrs %0, cpsr \n\t"
               "mrs %1, spsr \n\t"
               : "=r"(cpsr), "=r"(spsr)::);

  kprintf("\n>>> Aktuelle Statusregister (SPSR des aktuellen Modus) <<<\n");
  kprintf("CPSR: ");
  print_register_using_layout(cpsr, layout);
  kprintf(" (%#010x)\n", cpsr);

  kprintf("SPSR: ");
  print_register_using_layout(spsr, layout);
  kprintf(" (%#010x)\n", spsr);
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

void print_various_mode_registers(register_layout_part *layout) {
  static const cpu_mode modes[NUMBER_OF_MODES] = {m_system, m_supervisor,
                                                  m_abort, m_irq, m_undefined};
  uint32_t mode_regs[NUMBER_OF_MODES * 3];

  // FIXME: rausfinden wie man clangd dazu bringt Fehlermeldung zu ignorieren
  GET_MODE_REGS(m_system, &mode_regs[0]);
  GET_MODE_REGS(m_supervisor, &mode_regs[3]);
  GET_MODE_REGS(m_abort, &mode_regs[6]);
  GET_MODE_REGS(m_irq, &mode_regs[9]);
  GET_MODE_REGS(m_undefined, &mode_regs[12]);

  kprintf("\n>>> Aktuelle modusspezifische Register <<<\n");
  kprintf("             LR         SP         SPSR\n");

  for (size_t mode_index = 0; mode_index < NUMBER_OF_MODES; mode_index++) {
    uint32_t m_spsr = mode_regs[3 * mode_index + 0];
    uint32_t m_lr = mode_regs[3 * mode_index + 1];
    uint32_t m_sp = mode_regs[3 * mode_index + 2];
    kprintf("%s %#010x %#010x ", get_padded_mode_label(modes[mode_index]), m_lr,
            m_sp);
    print_register_using_layout(m_spsr, layout);
    kprintf(" (%#010x)\n", m_spsr);
  }

  kprintf("\n");
}

void dump_registers(uint32_t *regs) {
  print_general_registers(regs);

  register_layout_part layout[5] = {
      {.mnemonic = "NZCV", .last_member_bit_offset = 3},
      {.mnemonic = NULL, .last_member_bit_offset = 26},
      {.mnemonic = "E", .last_member_bit_offset = 27},
      {.mnemonic = NULL, .last_member_bit_offset = 28},
      {.mnemonic = "IFT", .last_member_bit_offset = 31}};

  print_current_mode_status_registers(layout);
  print_various_mode_registers(layout);
}