#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Userthread"

#define NUM_CALCULATION_CYCLES 20
#define SLEEP_TIME_MS 1000

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

  if (ch == 's') {
    // random bullshit go
    asm volatile("svc #0x1337 \t\n" ::
                     :);
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