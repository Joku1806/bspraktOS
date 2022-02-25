#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "Userthread"

#define GLOBAL_COUNTER_LIMIT 110
#define SLEEP_TIME_MS 2000

#include <config.h>
#include <stddef.h>
#include <stdint.h>
#include <user/lib/debug.h>
#include <user/lib/error.h>
#include <user/lib/syscall.h>
#include <user/main.h>

volatile char global_ch;
volatile size_t global_counter = 100;

void thread_main() {
  size_t local_counter = 0;

  while (global_counter < GLOBAL_COUNTER_LIMIT) {
    global_counter++;
    local_counter++;
    printf("%c:%u (%u:%u)\n", global_ch, global_counter, sys$get_thread_id(), local_counter);
    sys$stall_thread(SLEEP_TIME_MS);
  }
}

void process_main(void *args) {
  global_ch = *(char *)args;

  int ret;
  if ((ret = sys$create_thread(thread_main, NULL, 0)) < 0) {
    warnln("Couldn't create thread. Reason: %s", error_string(ret));
  }

  if ((ret = sys$create_thread(thread_main, NULL, 0)) < 0) {
    warnln("Couldn't create thread. Reason: %s", error_string(ret));
  }
}

void user_main() {
  int ret;

  while (true) {
    char ch = sys$read_character();
    if ((ret = sys$create_process(process_main, &ch, sizeof(ch))) < 0) {
      warnln("Couldn't create process. Reason: %s", error_string(ret));
    }
  }
}