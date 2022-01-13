#ifndef KFRACTION_H
#define KFRACTION_H

#include <stdbool.h>

typedef struct {
  long numerator;
  long denominator;
} k_fraction;

k_fraction k_fraction_create(long a, long b);
#define k_fraction_from_number(a) k_fraction_create(a, 1);
long k_fraction_to_whole_number(k_fraction *a);

bool k_fraction_lt(k_fraction *a, k_fraction *b);
bool k_fraction_eq(k_fraction *a, k_fraction *b);

k_fraction k_fraction_add(k_fraction *a, k_fraction *b);
k_fraction k_fraction_sub(k_fraction *a, k_fraction *b);
k_fraction k_fraction_multiply(k_fraction *a, k_fraction *b);

k_fraction k_fraction_floor(k_fraction *a);
k_fraction k_fraction_round(k_fraction *a);

k_fraction k_fraction_project_onto_range(k_fraction *a, k_fraction *min, k_fraction *max);

// FIXME: sollte woanders hin, vielleicht eigene Datei kfraction_helpers.h?
long k_project_long_onto_range(long a, long min, long max);

#endif