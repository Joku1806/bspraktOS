#ifndef KTIMING_H
#define KTIMING_H

#include <stddef.h>

size_t ktiming_milliseconds_to_hertz(size_t ms, int *error);
size_t ktiming_hertz_to_milliseconds(size_t hz);

#endif