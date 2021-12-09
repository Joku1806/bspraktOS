#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Interrupt"

#include <arch/bsp/interrupt_peripherals.h>
#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/dfsr.h>
#include <arch/cpu/ifsr.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <kernel/scheduler.h>
#include <kernel/thread.h>
#include <lib/assertions.h>
#include <lib/string.h>
#include <stdint.h>

extern bool print_registers;

void print_register_using_layout(uint32_t reg, register_layout_part *layout);
void print_current_mode_status_registers(register_layout_part *layout);
void print_general_registers(uint32_t *regs);
void print_various_mode_registers(register_layout_part *layout);
void dump_registers(uint32_t *regs);

void reset_interrupt_handler(uint32_t *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
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
  uint32_t ifsr = 0, ifar = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 1 \n\t"
               "mrc p15, 0, %1, c6, c0, 2 \n\t"
               : "=r"(ifsr), "=r"(ifar));
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Prefetch Abort an Adresse %#010x\n", regs[LR_POSITION]);
  kprintf("Zugriff: Ausführung von Instruktion an Adresse %#010x\n", ifar);
  kprintf("Fehler: %s\n", get_prefetch_abort_error_type(ifsr));

  dump_registers(regs);

  halt_cpu();
}

void data_abort_interrupt_handler(uint32_t *regs) {
  uint32_t dfsr = 0, dfar = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 0 \n\t"
               "mrc p15, 0, %1, c6, c0, 0 \n\t"
               : "=r"(dfsr), "=r"(dfar));
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Data Abort an Adresse %#010x\n", regs[LR_POSITION]);
  kprintf("Zugriff: %s auf Adresse %#010x\n", "lesend", dfar);
  kprintf("Fehler: %s\n", get_data_abort_error_type(dfsr));

  dump_registers(regs);

  halt_cpu();
}

void irq_interrupt_handler(uint32_t *regs) {
  if (print_registers) {
    kprintf("#############################################"
            "#######################"
            "#######\n");
    kprintf("IRQ Interrupt an Adresse %#010x\n", regs[LR_POSITION]);
    dump_registers(regs);
  }

  if (*peripherals_register(IRQ_pending_1) & timer1_pending) {
    schedule_thread(regs);
    systimer_reset();
  } else if (*peripherals_register(IRQ_pending_2) & UART_pending) {
    pl001_receive();
  }
}

void print_general_registers(uint32_t *regs) {
  kprintf("\n>>> Registerschnappschuss (aktueller Modus) <<<\n");
  static const char *aliases[3] = {"SP", "LR", "PC"};
  for (size_t reg_idx = 0; reg_idx < 8; reg_idx++) {
    kprintf("R%u: %#010x\t", reg_idx, regs[reg_idx]);

    if (reg_idx + 8 >= SP_POSITION && reg_idx + 8 <= PC_POSITION) {
      kprintf("%s: %#010x\n", aliases[reg_idx + 8 - SP_POSITION],
              regs[reg_idx + 8]);
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