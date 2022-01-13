#ifndef TIMING_H
#define TIMING_H

#include <stddef.h>

#define TIMER_FREQUENCY_HZ 1000000

size_t milliseconds_to_mhz(size_t ms);
void sleep_milliseconds(size_t ms);
void sleep_mhz(size_t mhz);
void sleep_macgyver(size_t instrs);
size_t mhz_to_milliseconds(size_t mhz);

#endif