#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Timing"

#include <arch/bsp/systimer.h>
#include <kernel/lib/ktiming.h>
#include <stddef.h>

size_t k_milliseconds_to_mhz(size_t ms) {
  return ms * SYSTIMER_FREQUENCY_HZ / 1000;
}

void k_sleep_milliseconds(size_t ms) {
  size_t initial = systimer_value();
  while (systimer_value() < initial + k_milliseconds_to_mhz(ms)) {}
}

void k_sleep_mhz(size_t mhz) {
  size_t initial = systimer_value();
  while (systimer_value() < initial + mhz) {}
}

void k_sleep_macgyver(size_t instrs) {
  for (volatile size_t i = 0; i < instrs; i++) {
    asm volatile("" ::
                     : "memory");
  }
}

size_t k_mhz_to_milliseconds(size_t mhz) {
  return (1000 * mhz) / SYSTIMER_FREQUENCY_HZ;
}