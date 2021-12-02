#include <arch/bsp/systimer.h>
#include <lib/timing.h>

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