#include <arch/bsp/systimer.h>
#include <lib/timing.h>

void sleep_mhz(size_t mhz) {
  size_t initial = systimer_value();
  while (systimer_value() < initial + mhz) {}
}