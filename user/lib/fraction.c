#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Fraction"
#define LOG_COLORED_OUTPUT false

#include <limits.h>
#include <user/lib/assertions.h>
#include <user/lib/debug.h>
#include <user/lib/error.h>
#include <user/lib/fraction.h>
#include <user/lib/math.h>

void fraction_compact(fraction *a);
int fraction_scalar_multiply(fraction *a, long scalar, fraction *out);

fraction fraction_create(long a, long b) {
  VERIFY(b != 0);

  fraction f = {.numerator = a, .denominator = b};
  fraction_compact(&f);

  return f;
}

long fraction_to_whole_number(fraction *a) {
  return a->numerator / a->denominator;
}

void fraction_compact(fraction *a) {
  long factor = gcd(a->numerator, a->denominator);
  a->numerator /= factor;
  a->denominator /= factor;
}

bool fraction_downcast_precision(fraction *a) {
  if (a->denominator == 1) {
    dbgln("Cannot downcast because fraction %i/%i is whole number.", a->numerator, a->denominator);
    return false;
  }

  if (a->numerator % 2 != 0) {
    a->numerator--;
  }

  if (a->denominator % 2 != 0) {
    a->denominator--;
  }

  fraction_compact(a);
  dbgln("Downcasted fraction precision because of certain overflow, is now %i/%i.", a->numerator, a->denominator);
  return true;
}

bool fraction_lt(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);
  dbgln("%i/%i < %i/%i : %s", a->numerator, a->denominator, b->numerator, b->denominator, a->numerator * (common / a->denominator) < b->numerator * (common / b->denominator) ? "true" : "false");

  return a->numerator * (common / a->denominator) < b->numerator * (common / b->denominator);
}

bool fraction_eq(fraction *a, fraction *b) {
  long common = lcm(a->denominator, b->denominator);
  dbgln("%i/%i == %i/%i : %s", a->numerator, a->denominator, b->numerator, b->denominator, a->numerator * (common / a->denominator) == b->numerator * (common / b->denominator) ? "true" : "false");

  return a->numerator * (common / a->denominator) == b->numerator * (common / b->denominator);
}

fraction fraction_add(fraction *a, fraction *b) {
  fraction a_adj = *a;
  fraction b_adj = *b;
  long res_num_add = 0;

  while (a_adj.denominator != b_adj.denominator || __builtin_add_overflow(a_adj.numerator, b_adj.numerator, &res_num_add)) {
    long common_multiple = lcm(a_adj.denominator, b_adj.denominator);
    while (fraction_scalar_multiply(&a_adj, common_multiple / a_adj.denominator, &a_adj) < 0 ||
           fraction_scalar_multiply(&b_adj, common_multiple / b_adj.denominator, &b_adj) < 0) {
      fraction_compact(&a_adj);
      fraction_compact(&b_adj);

      if (!fraction_downcast_precision(&a_adj) && !fraction_downcast_precision(&b_adj)) {
        // FIXME: Stattdessen Fehlercode zurückgeben
        panicln("Can't avoid overflow when computing %i/%i + %i/%i.", a_adj.numerator, a_adj.denominator, b_adj.numerator, b_adj.denominator);
      }

      common_multiple = lcm(a_adj.denominator, b_adj.denominator);
    }

    while (a_adj.denominator == b_adj.denominator && __builtin_add_overflow(a_adj.numerator, b_adj.numerator, &res_num_add)) {
      if (!fraction_downcast_precision(&a_adj) && !fraction_downcast_precision(&b_adj)) {
        panicln("Can't avoid overflow when computing %i/%i + %i/%i.", a_adj.numerator, a_adj.denominator, b_adj.numerator, b_adj.denominator);
      }
    }
  }

  fraction ret = fraction_create(res_num_add, a_adj.denominator);
  dbgln("%i/%i + %i/%i = %i/%i", a->numerator, a->denominator, b->numerator, b->denominator, ret.numerator, ret.denominator);

  return ret;
}

void fraction_change_sign(fraction *a) {
  if (a->denominator < 0) {
    a->denominator = -a->denominator;
  } else {
    a->numerator = -a->numerator;
  }
}

fraction fraction_sub(fraction *a, fraction *b) {
  fraction b_negative = *b;
  fraction_change_sign(&b_negative);
  return fraction_add(a, &b_negative);
}

fraction fraction_multiply(fraction *a, fraction *b) {
  fraction a_cpy = *a;
  fraction b_cpy = *b;
  long num_mul = 0, denom_mul = 0;
  while (__builtin_mul_overflow(a_cpy.numerator, b_cpy.numerator, &num_mul) ||
         __builtin_mul_overflow(a_cpy.denominator, b_cpy.denominator, &denom_mul)) {
    if (!fraction_downcast_precision(&a_cpy) && !fraction_downcast_precision(&b_cpy)) {
      panicln("Can't avoid overflow when computing %i/%i * %i/%i.", a_cpy.numerator, a_cpy.denominator, b_cpy.numerator, b_cpy.denominator);
    }
  }

  fraction ret = fraction_create(num_mul, denom_mul);
  dbgln("%i/%i * %i/%i = %i/%i", a->numerator, a->denominator, b->numerator, b->denominator, ret.numerator, ret.denominator);

  return ret;
}

int fraction_scalar_multiply(fraction *a, long scalar, fraction *out) {
  *out = *a;

  if (scalar == 0) {
    out->numerator = 0;
    return 0;
  }

  long num_mul = 0, denom_mul = 0;
  if (__builtin_mul_overflow(out->numerator, scalar, &num_mul) ||
      __builtin_mul_overflow(out->denominator, scalar, &denom_mul)) {
    return -ERANGE;
  }

  out->numerator = num_mul;
  out->denominator = denom_mul;
  dbgln("%i/%i scaled by %i = %i/%i", a->numerator, a->denominator, scalar, out->numerator, out->denominator);

  return 0;
}

fraction fraction_floor(fraction *a) {
  fraction ret = fraction_create(a->numerator - a->numerator % a->denominator, a->denominator);
  dbgln("Flooring %i/%i -> %i/%i", a->numerator, a->denominator, ret.numerator, ret.denominator);
  return ret;
}

fraction fraction_round(fraction *a) {
  fraction half = fraction_create(1, 2);
  fraction added = fraction_add(a, &half);
  fraction ret = fraction_floor(&added);
  dbgln("Rounding %i/%i -> %i/%i", a->numerator, a->denominator, ret.numerator, ret.denominator);
  return ret;
}

fraction fraction_project_onto_range(fraction *a, fraction *min, fraction *max) {
  VERIFY(a->numerator <= a->denominator);
  VERIFY(!fraction_lt(max, min));

  fraction diff = fraction_sub(max, min);
  fraction scaled = fraction_multiply(a, &diff);
  fraction ret = fraction_add(&scaled, min);
  dbgln("Projecting %i/%i onto [%i/%i, %i/%i] -> %i/%i", a->numerator, a->denominator, min->numerator, min->denominator, max->numerator, max->denominator, ret.numerator, ret.denominator);
  return ret;
}

long project_long_onto_range(long a, long min, long max) {
  fraction f_scale = fraction_create(a, LONG_MAX);
  fraction f_min = fraction_from_number(min);
  fraction f_max = fraction_from_number(max);
  fraction f_projected = fraction_project_onto_range(&f_scale, &f_min, &f_max);
  fraction f_rounded = fraction_round(&f_projected);
  return fraction_to_whole_number(&f_rounded);
}