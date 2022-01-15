#include <stdint.h>
#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Timing"

#include <stddef.h>
#include <user/lib/syscall.h>
#include <user/lib/timing.h>

void sleep_milliseconds(size_t ms) {
  uint32_t initial = sys$get_time();
  while (sys$get_time() < initial + ms) {}
}

void sleep_macgyver(size_t instrs) {
  for (volatile size_t i = 0; i < instrs; i++) {
    asm volatile("" ::
                     : "memory");
  }
}