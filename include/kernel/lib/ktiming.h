#ifndef KTIMING_H
#define KTIMING_H

#include <stddef.h>

// FIXME: Braucht der Kernel überhaupt sleep-Funktionen?
size_t k_milliseconds_to_mhz(size_t ms);
void k_sleep_milliseconds(size_t ms);
void k_sleep_mhz(size_t mhz);
void k_sleep_macgyver(size_t instrs);
size_t k_mhz_to_milliseconds(size_t mhz);

#endif