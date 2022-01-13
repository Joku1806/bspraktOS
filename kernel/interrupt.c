#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Interrupt"

#include <arch/bsp/peripherals.h>
#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/dfsr.h>
#include <arch/cpu/ifsr.h>
#include <arch/cpu/mission_control.h>
#include <arch/cpu/psr.h>
#include <kernel/interrupt.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kerror.h>
#include <kernel/lib/kintrusive_list.h>
#include <kernel/lib/kprintf.h>
#include <kernel/lib/kstring.h>
#include <kernel/lib/ktiming.h>
#include <kernel/register_dump.h>
#include <kernel/scheduler.h>
#include <kernel/syscall_impl.h>
#include <stdint.h>
#include <user/main.h>

void reset_interrupt_handler(registers *regs) {
  dump_registers(reset, regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    kpanicln("Got reset interrupt in kernel space.");
  }
}

void undefined_instruction_interrupt_handler(registers *regs) {
  dump_registers(undefined_instruction, regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    kpanicln("Got undefined instruction interrupt in kernel space.");
  }
}

void software_interrupt_handler(registers *regs) {
  // FIXME: Sollte wahrscheinlich nach kernel/syscall_impl
  if ((get_spsr() & psr_mode) == psr_mode_user) {
    void *svc_address = (char *)(regs->lr) - 4;
    if (is_valid_syscall(svc_address) && dispatch_syscall(regs, get_syscall_no(svc_address)) >= 0) {
      return;
    }
  }

  dump_registers(software, regs);

  if ((get_spsr() & psr_mode) != psr_mode_user) {
    kpanicln("Got software interrupt in kernel space.");
  } else {
    kwarnln("User Thread tried to call unknown or malformed syscall.");
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  }
}

void prefetch_abort_interrupt_handler(registers *regs) {
  dump_registers(prefetch_abort, regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    kpanicln("Got prefetch abort interrupt in kernel space.");
  }
}

void data_abort_interrupt_handler(registers *regs) {
  dump_registers(data_abort, regs);

  if ((get_spsr() & psr_mode) == psr_mode_user) {
    VERIFY(dispatch_syscall(regs, SYSCALL_EXIT_THREAD_NO) >= 0);
  } else {
    kpanicln("Got data abort interrupt in kernel space.");
  }
}

void irq_interrupt_handler(registers *regs) {
  if (*peripherals_register(IRQ_pending_1) & timer1_pending) {
    scheduler_unblock_stall_waiting_threads(systimer_value());
    scheduler_round_robin(regs);
    systimer_reset();
  } else if (*peripherals_register(IRQ_pending_2) & UART_pending) {
    pl001_receive();
  }

  while (scheduler_exists_input_waiting_thread() && pl001_has_unread_character()) {
    scheduler_unblock_first_input_waiting_thread(pl001_read());
  }
}