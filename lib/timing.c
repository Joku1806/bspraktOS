#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Timing"

#include <arch/bsp/systimer.h>
#include <lib/timing.h>
#include <stddef.h>

size_t milliseconds_to_mhz(size_t ms) {
  return ms * SYSTIMER_FREQUENCY_HZ / 1000;
}

void sleep_milliseconds(size_t ms) {
  size_t initial = systimer_value();
  while (systimer_value() < initial + milliseconds_to_mhz(ms)) {}
}

void sleep_mhz(size_t mhz) {
  size_t initial = systimer_value();
  while (systimer_value() < initial + mhz) {}
}

void sleep_macgyver(size_t instrs) {
  for (volatile size_t i = 0; i < instrs; i++) {
    asm volatile("" ::
                     : "memory");
  }
}