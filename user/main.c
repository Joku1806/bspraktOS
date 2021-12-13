#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Userthread"

#define NUM_CALCULATION_CYCLES 50

#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <lib/debug.h>
#include <lib/timing.h>
#include <stddef.h>
#include <user/main.h>

void main(void *x) {
  char ch = *(char *)x;
  dbgln("Successfully started thread with char argument '%c'!", ch);

  switch (ch) {
    case 'a':
      asm volatile("mov r0, #0x1 \n ldr r0, [r0]");
      break;
    case 'p':
      asm volatile("bkpt #0");
      break;
    case 's':
      asm volatile("svc #1337");
      break;
    case 'u':
      asm volatile(".word 0xf7f0a000\n");
      break;
    case 'c':
      register_checker();
      break;
    default:
      for (size_t cycles = 0; cycles < NUM_CALCULATION_CYCLES; cycles++) {
        sleep_macgyver(BUSY_WAIT_COUNTER);
        kprintf("%c", ch);
      }
  }
}
