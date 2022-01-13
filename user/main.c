#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Userthread"

#define NUM_CALCULATION_CYCLES 20

#include <config.h>
#include <stddef.h>
#include <user/lib/ascii.h>
#include <user/lib/debug.h>
#include <user/lib/error.h>
#include <user/lib/syscall.h>
#include <user/lib/timing.h>
#include <user/main.h>

void printer(void *x) {
  char ch = *(char *)x;
  dbgln("Successfully started user thread with char argument '%c'!", ch);

  for (int i = 0; i < NUM_CALCULATION_CYCLES; i++) {
    sys$output_character(ch);

    if (is_uppercase(ch)) {
      sleep_macgyver(BUSY_WAIT_COUNTER);
    } else {
      sys$stall_thread(mhz_to_milliseconds(BUSY_WAIT_COUNTER));
    }
  }
}

void user_main() {
  while (true) {
    char ch = sys$read_character();
    sys$create_thread(printer, &ch, sizeof(ch));
  }
}