#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <kernel/lib/kerror.h>
#include <kernel/lib/ktiming.h>
#include <kernel/scheduler.h>
#include <kernel/syscall_impl.h>
#include <stdint.h>

uint32_t get_syscall_no(void *svc_instruction_address) {
  return *(uint32_t *)svc_instruction_address & 0x00ffffff;
}

bool is_valid_syscall_no(uint32_t syscall_no) {
  return syscall_no == SYSCALL_READ_CHARACTER_NO ||
         syscall_no == SYSCALL_OUTPUT_CHARACTER_NO ||
         syscall_no == SYSCALL_CREATE_THREAD_NO ||
         syscall_no == SYSCALL_STALL_THREAD_NO ||
         syscall_no == SYSCALL_EXIT_THREAD_NO;
}

bool is_valid_syscall(void *instruction_address) {
  return (*(uint32_t *)instruction_address & 0xff000000) == 0xef000000 &&
         is_valid_syscall_no(get_syscall_no(instruction_address));
}

int dispatch_syscall(registers *regs, uint32_t syscall_no) {
  switch (syscall_no) {
    // FIXME: erster Thread in Liste kriegt erstes Zeichen,
    // es sollen nicht alle Threads in der Liste das erste
    // Zeichen kriegen.
    case SYSCALL_READ_CHARACTER_NO: {
      tcb *calling_thread = scheduler_get_running_thread();
      scheduler_forced_round_robin(regs);
      scheduler_ignore_thread_until_character_input(calling_thread);
      return 0;
    }

    case SYSCALL_OUTPUT_CHARACTER_NO: {
      pl001_send(regs->general[0]);
      return 0;
    }

    case SYSCALL_CREATE_THREAD_NO: {
      void *func = (void *)regs->general[0];
      void *args = (void *)regs->general[1];
      unsigned int args_size = regs->general[2];

      if (!scheduler_is_thread_available()) {
        regs->general[0] = -K_EBUSY;
        return 0;
      }

      scheduler_create_thread(func, args, args_size);
      return 0;
    }

    // FIXME: sleep(0) separat behandeln, nicht schlafen legen, nur ans Ende der ready-Liste
    case SYSCALL_STALL_THREAD_NO: {
      unsigned ms = regs->general[0];
      // WICHTIG: Die Reihenfolge hier nicht ändern, ansonsten wird der
      // Kontext vom aufrufenden Thread nicht gespeichert!
      // FIXME: Sollte nicht von der Reihenfolge abhängig sein, vielleicht
      // können wir den Threadkontext explizit sichern?
      tcb *calling_thread = scheduler_get_running_thread();
      scheduler_forced_round_robin(regs);
      scheduler_ignore_thread_until_timer_match(calling_thread, systimer_value() + k_milliseconds_to_mhz(ms));
      systimer_reset();
      return 0;
    }

    case SYSCALL_EXIT_THREAD_NO: {
      scheduler_cleanup_thread();
      scheduler_round_robin(regs);
      systimer_reset();
      return 0;
    }

    case SYSCALL_GET_TIME_NO: {
      regs->general[0] = systimer_value();
      return 0;
    }
  }

  return -K_EINVAL;
}