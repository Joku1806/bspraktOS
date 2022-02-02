#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Userthread"

#define NUM_CALCULATION_CYCLES 20
#define SLEEP_TIME_MS 2000

#include <arch/bsp/pl001.h>
#include <arch/bsp/stack_defines.h>
#include <config.h>
#include <stddef.h>
#include <stdint.h>
#include <user/lib/ascii.h>
#include <user/lib/debug.h>
#include <user/lib/error.h>
#include <user/lib/syscall.h>
#include <user/lib/timing.h>
#include <user/main.h>

void printer(void *x) {
  volatile unsigned dummy;
  char ch = *(char *)x;
  dbgln("Successfully started user thread with char argument '%c'!", ch);

  switch (ch) {
    case 'n':
      dummy = *(volatile unsigned *)0;
      break;
    case 'p':
      asm volatile("mov pc, #0");
      break;
    case 'd':
      dummy = *(volatile unsigned *)0x00050000;
      break;
    case 'k':
      dummy = *(volatile unsigned *)0x00010000;
      break;
    case 'K':
      dummy = *(volatile unsigned *)IRQ_SP;
      break;
    case 'g':
      dummy = *(volatile unsigned *)UART_BASE;
      break;
    case 'c':
      *(volatile unsigned *)0x00080000 = 0x1337;
      break;
    case 's':
      *(volatile unsigned *)(&x + 0x00200000) = 0x1337;
      break;
    case 'u':
      // FIXME: heißt Zugriff auf Adresse > 128 MiB
      dummy = *(volatile unsigned *)(&x + 0x00200000);
      break;
    case 'x':
      asm volatile("mov pc, %0" ::"r"(&x));
      break;
  }

  for (int i = 0; i < NUM_CALCULATION_CYCLES; i++) {
    sys$output_character(ch);

    if (is_uppercase(ch)) {
      sleep_milliseconds(SLEEP_TIME_MS);
    } else {
      sys$stall_thread(SLEEP_TIME_MS);
    }
  }
}

void user_main() {
  int ret;

  while (true) {
    char ch = sys$read_character();
    if ((ret = sys$create_thread(printer, &ch, sizeof(ch))) < 0) {
      warnln("Couldn't create thread. Reason: %s", error_string(ret));
    }
  }
}