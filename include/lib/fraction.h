#ifndef FRACTION_H
#define FRACTION_H

#include <stdbool.h>

typedef struct {
  long numerator;
  long denominator;
} fraction;

fraction fraction_create(long a, long b);
fraction fraction_create_from_whole_number(long a);
long fraction_to_whole_number(fraction *a);

bool fraction_lt(fraction *a, fraction *b);
bool fraction_eq(fraction *a, fraction *b);

fraction fraction_add(fraction *a, fraction *b);
fraction fraction_sub(fraction *a, fraction *b);
fraction fraction_multiply(fraction *a, fraction *b);

fraction fraction_floor(fraction *a);
fraction fraction_round(fraction *a);

fraction fraction_project_onto_range(fraction *a, fraction *min, fraction *max);

#endif