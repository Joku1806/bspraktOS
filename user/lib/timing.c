#include <stdint.h>
#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Timing"

#include <stddef.h>
#include <user/lib/syscall.h>
#include <user/lib/timing.h>

size_t milliseconds_to_mhz(size_t ms) {
  return ms * TIMER_FREQUENCY_HZ / 1000;
}

// FIXME: verlieren wir hier zu viel Genauigkeit?
size_t mhz_to_milliseconds(size_t mhz) {
  return (1000 * mhz) / TIMER_FREQUENCY_HZ;
}

void sleep_milliseconds(size_t ms) {
  // FIXME: nachschauen ob 32bit auslangen oder ob man am besten die vollen
  // 64bit benutzen sollte.
  uint32_t initial = sys$get_time();
  while (sys$get_time() < initial + milliseconds_to_mhz(ms)) {}
}

void sleep_mhz(size_t mhz) {
  uint32_t initial = sys$get_time();
  while (sys$get_time() < initial + mhz) {}
}

void sleep_macgyver(size_t instrs) {
  for (volatile size_t i = 0; i < instrs; i++) {
    asm volatile("" ::
                     : "memory");
  }
}