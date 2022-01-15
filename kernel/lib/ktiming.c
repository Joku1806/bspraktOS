#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Timing"

#include <arch/bsp/systimer.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kfraction.h>
#include <kernel/lib/ktiming.h>
#include <stddef.h>

size_t ktiming_milliseconds_to_hertz(size_t ms, int *error) {
  k_fraction s = k_fraction_create(ms, 1000);
  k_fraction freq = k_fraction_create(SYSTIMER_FREQUENCY_HZ, 1);

  k_fraction hz = k_fraction_multiply(&s, &freq, error);
  if (*error < 0) {
    kwarnln("%ums can't be converted to systimer hertz units.", ms);
    return 0;
  }

  return k_fraction_to_whole_number(&hz);
}

// ms = hz / freq * 1000
size_t ktiming_hertz_to_milliseconds(size_t hz) {
  k_fraction s = k_fraction_create(hz, SYSTIMER_FREQUENCY_HZ);
  k_fraction thousand = k_fraction_create(1000, 1);
  k_fraction ms = k_fraction_multiply(&s, &thousand, &(int){0});

  return k_fraction_to_whole_number(&ms);
}