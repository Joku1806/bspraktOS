#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Fraction"
#define LOG_COLORED_OUTPUT false

#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/fraction.h>
#include <lib/math.h>

fraction fraction_create(long a, long b) {
  VERIFY(b != 0);

  fraction f = {.numerator = a, .denominator = b};
  long factor = gcd(a, b);
  f.numerator /= factor;
  f.denominator /= factor;

  return f;
}

fraction fraction_create_from_whole_number(long a) {
  fraction f = {.numerator = a, .denominator = 1};
  return f;
}

long fraction_to_whole_number(fraction *a) {
  return a->numerator / a->denominator;
}

bool fraction_lt(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);

  return a->numerator * (common / a->denominator) < b->numerator * (common / b->denominator);
}

bool fraction_eq(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);

  return a->numerator * (common / a->denominator) == b->numerator * (common / b->denominator);
}

fraction fraction_add(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);

  return fraction_create(a->numerator * (common / a->denominator) + b->numerator * (common / b->denominator), common);
}

fraction fraction_sub(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);

  return fraction_create(a->numerator * (common / a->denominator) - b->numerator * (common / b->denominator), common);
}

fraction fraction_multiply(fraction *a, fraction *b) {
  return fraction_create(a->numerator * b->numerator, a->denominator * b->denominator);
}

fraction fraction_floor(fraction *a) {
  return fraction_create(a->numerator - a->numerator % a->denominator, a->denominator);
}

fraction fraction_round(fraction *a) {
  fraction half = fraction_create(1, 2);
  fraction added = fraction_add(a, &half);
  return fraction_floor(&added);
}

fraction fraction_project_onto_range(fraction *a, fraction *min, fraction *max) {
  VERIFY(a->numerator <= a->denominator);
  VERIFY(!fraction_lt(max, min));

  fraction diff = fraction_sub(max, min);
  fraction scaled = fraction_multiply(a, &diff);
  return fraction_add(&scaled, min);
}