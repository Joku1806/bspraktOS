#include <arch/cpu/dfsr.h>
#include <arch/cpu/ifsr.h>
#include <arch/cpu/psr.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kprintf.h>
#include <kernel/lib/kstring.h>
#include <kernel/register_dump.h>

void print_general_registers(registers *regs) {
  kprintf("\n>>> Registerschnappschuss (aktueller Modus) <<<\n");
  static const char *aliases[3] = {"SP", "LR", "PC"};
  uint32_t *regs_iter = (uint32_t *)regs;

  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    kprintf("R%u: %#010x\t", reg_idx, regs_iter[reg_idx]);

    if (reg_idx + 8 >= SP_POSITION && reg_idx + 8 <= PC_POSITION) {
      kprintf("%s: %#010x\n", aliases[reg_idx + 8 - SP_POSITION],
              regs_iter[reg_idx + 8]);
    } else if (reg_idx + 8 != 16) {
      kprintf("R%u: %#010x\n", reg_idx + 8, regs_iter[reg_idx + 8]);
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
      size_t flag_index = k_strlen(layout->mnemonic) -
                          (layout->last_member_bit_offset - bit_offset) - 1;
      VERIFY(flag_index < k_strlen(layout->mnemonic));
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
  kprintf(" %s (%#010x)\n", get_mode_name(cpsr), cpsr);

  kprintf("SPSR: ");
  print_register_using_layout(spsr, layout);
  kprintf(" %s (%#010x)\n", get_mode_name(spsr), spsr);
}

void print_various_mode_registers(register_layout_part *layout) {
  static const psr_mode_bits modes[NUMBER_OF_USED_MODES] = {
      psr_mode_system, psr_mode_supervisor, psr_mode_abort, psr_mode_IRQ,
      psr_mode_undefined};
  uint32_t mode_regs[NUMBER_OF_USED_MODES * 3];

  GET_MODE_REGS(psr_mode_system, &mode_regs[0]);
  GET_MODE_REGS(psr_mode_supervisor, &mode_regs[3]);
  GET_MODE_REGS(psr_mode_abort, &mode_regs[6]);
  GET_MODE_REGS(psr_mode_IRQ, &mode_regs[9]);
  GET_MODE_REGS(psr_mode_undefined, &mode_regs[12]);

  kprintf("\n>>> Aktuelle modusspezifische Register <<<\n");
  kprintf("           LR         SP         SPSR\n");

  for (size_t mode_index = 0; mode_index < NUMBER_OF_USED_MODES; mode_index++) {
    uint32_t m_spsr = mode_regs[3 * mode_index + 0];
    uint32_t m_lr = mode_regs[3 * mode_index + 1];
    uint32_t m_sp = mode_regs[3 * mode_index + 2];
    // FIXME: geht das Padding irgendwie mit Variablen/Macros?
    kprintf("%10s %#010x %#010x ", get_mode_name(modes[mode_index]), m_lr,
            m_sp);

    if (modes[mode_index] != psr_mode_system) {
      print_register_using_layout(m_spsr, layout);
      kprintf(" %s (%#010x)", get_mode_name(m_spsr), m_spsr);
    }

    kprintf("\n");
  }

  kprintf("\n");
}

char *get_interrupt_type_name(interrupt_type type) {
  switch (type) {
    case reset:
      return "Reset";
    case undefined_instruction:
      return "Undefined Instruction";
    case software:
      return "Software";
    case prefetch_abort:
      return "Prefetch Abort";
    case data_abort:
      return "Data Abort";
    case irq:
      return "IRQ";
    default:
      VERIFY_NOT_REACHED();
  }
}

void dump_registers(interrupt_type type, registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("%s Interrupt an Adresse %#010x\n", get_interrupt_type_name(type), regs->lr);

  if (type == prefetch_abort) {
    uint32_t ifsr = 0, ifar = 0;
    asm volatile("mrc p15, 0, %0, c5, c0, 1 \n\t"
                 "mrc p15, 0, %1, c6, c0, 2 \n\t"
                 : "=r"(ifsr), "=r"(ifar));

    kprintf("Zugriff: Ausführung von Instruktion an Adresse %#010x\n", ifar);
    kprintf("Fehler: %s\n", get_prefetch_abort_error_type(ifsr));
  } else if (type == data_abort) {
    uint32_t dfsr = 0, dfar = 0;
    asm volatile("mrc p15, 0, %0, c5, c0, 0 \n\t"
                 "mrc p15, 0, %1, c6, c0, 0 \n\t"
                 : "=r"(dfsr), "=r"(dfar));

    kprintf("Zugriff: %s auf Adresse %#010x\n", "lesend", dfar);
    kprintf("Fehler: %s\n", get_data_abort_error_type(dfsr));
  }

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