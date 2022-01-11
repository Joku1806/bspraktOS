#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Userthread"

#define NUM_CALCULATION_CYCLES 50

#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <kernel/syscall.h>
#include <lib/character_types.h>
#include <lib/debug.h>
#include <lib/timing.h>
#include <stddef.h>
#include <user/main.h>

void printer(void *x) {

  char ch = *(char *)x;
  dbgln("Successfully started user thread with char argument '%c'!", ch);

  for (int i = 0; i < 20; i++) {
    sys$output_character(ch);

    if (is_uppercase(ch)) {
      sleep_macgyver(BUSY_WAIT_COUNTER);
    } else {
      sys$stall_thread(mhz_to_milliseconds(BUSY_WAIT_COUNTER));
    }
  }
}

void main() {
  while (true) {
    char ch = sys$read_character();
    sys$create_thread(printer, &ch, sizeof(ch));
  }
}