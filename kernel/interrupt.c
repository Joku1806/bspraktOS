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
#include <kernel/syscall.h>
#include <lib/assertions.h>
#include <lib/error_codes.h>
#include <lib/intrusive_list.h>
#include <lib/string.h>
#include <lib/timing.h>
#include <stdint.h>
#include <user/main.h>

void populate_thread_pool();
int dispatch_syscall(registers *regs, uint32_t syscall_no);
void print_register_using_layout(uint32_t reg, register_layout_part *layout);
void print_current_mode_status_registers(register_layout_part *layout);
void print_general_registers(registers *regs);
void print_various_mode_registers(register_layout_part *layout);
void dump_registers(registers *regs);

void reset_interrupt_handler(registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Reset Interrupt an Adresse %#010x\n", regs->lr);
  dump_registers(regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    panicln("Got reset interrupt in kernel space.");
  }
}

void undefined_instruction_interrupt_handler(registers *regs) {
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Undefined Instruction an Adresse %#010x\n", regs->lr);
  dump_registers(regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    panicln("Got undefined instruction interrupt in kernel space.");
  }
}

int dispatch_syscall(registers *regs, uint32_t syscall_no) {
  tcb *calling_thread = scheduler_get_running_thread();

  switch (syscall_no) {
    case SYSCALL_READ_CHARACTER_NO: {
      schedule_thread(regs);
      scheduler_ignore_thread_until_character_input(calling_thread);
      return 0;
    }

    case SYSCALL_OUTPUT_CHARACTER_NO: {
      char ch = *(char *)regs->sp;
      pl001_send(ch);
      return 0;
    }

    case SYSCALL_CREATE_THREAD_NO: {
      void *func = *(void **)regs->sp;
      void *args = *(void **)((uint32_t)regs->sp + sizeof(void *));
      unsigned int args_size = *(unsigned int *)((uint32_t)regs->sp + sizeof(void *) + sizeof(void *));
      thread_create(func, args, args_size);
      return 0;
    }

    case SYSCALL_STALL_THREAD_NO: {
      unsigned ms = *(unsigned *)regs->sp;
      // WICHTIG: Die Reihenfolge hier nicht ändern, ansonsten wird der
      // Kontext vom aufrufenden Thread nicht gespeichert!
      // FIXME: Sollte nicht von der Reihenfolge abhängig sein, vielleicht
      // können wir den Threadkontext explizit sichern?
      schedule_thread(regs);
      scheduler_ignore_thread_until_timer_match(calling_thread, systimer_value() + milliseconds_to_mhz(ms));
      systimer_reset();
      return 0;
    }

    case SYSCALL_EXIT_THREAD_NO: {
      thread_cleanup();
      populate_thread_pool();
      schedule_thread(regs);
      systimer_reset();
      return 0;
    }
  }

  return -EINVAL;
}

void software_interrupt_handler(registers *regs) {
  // -4 um lr zu korrigieren
  void *svc_address = (char *)(regs->lr) - 4;
  if (is_syscall(svc_address) &&
      dispatch_syscall(regs, get_syscall_no(svc_address)) >= 0) {
    return;
  }

  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Software Interrupt an Adresse %#010x\n", regs->lr);
  dump_registers(regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    panicln("Got software interrupt in kernel space.");
  }
}

void prefetch_abort_interrupt_handler(registers *regs) {
  uint32_t ifsr = 0, ifar = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 1 \n\t"
               "mrc p15, 0, %1, c6, c0, 2 \n\t"
               : "=r"(ifsr), "=r"(ifar));
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Prefetch Abort an Adresse %#010x\n", regs->lr);
  kprintf("Zugriff: Ausführung von Instruktion an Adresse %#010x\n", ifar);
  kprintf("Fehler: %s\n", get_prefetch_abort_error_type(ifsr));

  dump_registers(regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    panicln("Got prefetch abort interrupt in kernel space.");
  }
}

void data_abort_interrupt_handler(registers *regs) {
  uint32_t dfsr = 0, dfar = 0;
  asm volatile("mrc p15, 0, %0, c5, c0, 0 \n\t"
               "mrc p15, 0, %1, c6, c0, 0 \n\t"
               : "=r"(dfsr), "=r"(dfar));
  kprintf("#############################################"
          "#######################"
          "#######\n");
  kprintf("Data Abort an Adresse %#010x\n", regs->lr);
  kprintf("Zugriff: %s auf Adresse %#010x\n", "lesend", dfar);
  kprintf("Fehler: %s\n", get_data_abort_error_type(dfsr));

  dump_registers(regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    panicln("Got data abort interrupt in kernel space.");
  }
}

void populate_thread_pool() {
  while (scheduler_is_thread_available() && pl001_has_unread_character()) {
    char ch = pl001_read();
    dbgln("Got character %c", ch);

    switch (ch) {
      case 'A':
        asm volatile("mov r0, #0x1 \n ldr r0, [r0]");
        break;
      case 'P':
        asm volatile("bkpt #0");
        break;
      case 'S':
        asm volatile("svc #1337");
        break;
      case 'U':
        asm volatile(".word 0xf7f0a000\n");
        break;
      default:
        thread_create(main, &ch, 1);
    }
  }
}

void irq_interrupt_handler(registers *regs) {
  if (*peripherals_register(IRQ_pending_1) & timer1_pending) {
    kprintf("!");
    scheduler_unblock_stall_waiting_threads(systimer_value());
    schedule_thread(regs);
    systimer_reset();
  } else if (*peripherals_register(IRQ_pending_2) & UART_pending) {
    pl001_receive();
    scheduler_unblock_input_waiting_threads(pl001_read());

    // populate_thread_pool();
  }
}

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

void dump_registers(registers *regs) {
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