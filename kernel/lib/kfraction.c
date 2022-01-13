#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Fraction"
#define LOG_COLORED_OUTPUT false

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kerror.h>
#include <kernel/lib/kfraction.h>
#include <kernel/lib/kmath.h>
#include <limits.h>

void k_fraction_compact(k_fraction *a);
int k_fraction_scalar_multiply(k_fraction *a, long scalar, k_fraction *out);

k_fraction k_fraction_create(long a, long b) {
  VERIFY(b != 0);

  k_fraction f = {.numerator = a, .denominator = b};
  k_fraction_compact(&f);

  return f;
}

long k_fraction_to_whole_number(k_fraction *a) {
  return a->numerator / a->denominator;
}

void k_fraction_compact(k_fraction *a) {
  long factor = k_gcd(a->numerator, a->denominator);
  a->numerator /= factor;
  a->denominator /= factor;
}

bool k_fraction_downcast_precision(k_fraction *a) {
  if (a->denominator == 1) {
    kdbgln("Cannot downcast because k_fraction %i/%i is whole number.", a->numerator, a->denominator);
    return false;
  }

  if (a->numerator % 2 != 0) {
    a->numerator--;
  }

  if (a->denominator % 2 != 0) {
    a->denominator--;
  }

  k_fraction_compact(a);
  kdbgln("Downcasted k_fraction precision because of certain overflow, is now %i/%i.", a->numerator, a->denominator);
  return true;
}

bool k_fraction_lt(k_fraction *a, k_fraction *b) {
  long common = k_lcm(a->denominator, b->denominator);
  kdbgln("%i/%i < %i/%i : %s", a->numerator, a->denominator, b->numerator, b->denominator, a->numerator * (common / a->denominator) < b->numerator * (common / b->denominator) ? "true" : "false");

  return a->numerator * (common / a->denominator) < b->numerator * (common / b->denominator);
}

bool k_fraction_eq(k_fraction *a, k_fraction *b) {
  long common = k_lcm(a->denominator, b->denominator);
  kdbgln("%i/%i == %i/%i : %s", a->numerator, a->denominator, b->numerator, b->denominator, a->numerator * (common / a->denominator) == b->numerator * (common / b->denominator) ? "true" : "false");

  return a->numerator * (common / a->denominator) == b->numerator * (common / b->denominator);
}

k_fraction k_fraction_add(k_fraction *a, k_fraction *b) {
  k_fraction a_adj = *a;
  k_fraction b_adj = *b;
  long res_num_add = 0;

  while (a_adj.denominator != b_adj.denominator || __builtin_add_overflow(a_adj.numerator, b_adj.numerator, &res_num_add)) {
    long common_multiple = k_lcm(a_adj.denominator, b_adj.denominator);
    while (k_fraction_scalar_multiply(&a_adj, common_multiple / a_adj.denominator, &a_adj) < 0 ||
           k_fraction_scalar_multiply(&b_adj, common_multiple / b_adj.denominator, &b_adj) < 0) {
      k_fraction_compact(&a_adj);
      k_fraction_compact(&b_adj);

      if (!k_fraction_downcast_precision(&a_adj) && !k_fraction_downcast_precision(&b_adj)) {
        // FIXME: Stattdessen Fehlercode zurückgeben
        kpanicln("Can't avoid overflow when computing %i/%i + %i/%i.", a_adj.numerator, a_adj.denominator, b_adj.numerator, b_adj.denominator);
      }

      common_multiple = k_lcm(a_adj.denominator, b_adj.denominator);
    }

    while (a_adj.denominator == b_adj.denominator && __builtin_add_overflow(a_adj.numerator, b_adj.numerator, &res_num_add)) {
      if (!k_fraction_downcast_precision(&a_adj) && !k_fraction_downcast_precision(&b_adj)) {
        kpanicln("Can't avoid overflow when computing %i/%i + %i/%i.", a_adj.numerator, a_adj.denominator, b_adj.numerator, b_adj.denominator);
      }
    }
  }

  k_fraction ret = k_fraction_create(res_num_add, a_adj.denominator);
  kdbgln("%i/%i + %i/%i = %i/%i", a->numerator, a->denominator, b->numerator, b->denominator, ret.numerator, ret.denominator);

  return ret;
}

void k_fraction_change_sign(k_fraction *a) {
  if (a->denominator < 0) {
    a->denominator = -a->denominator;
  } else {
    a->numerator = -a->numerator;
  }
}

k_fraction k_fraction_sub(k_fraction *a, k_fraction *b) {
  k_fraction b_negative = *b;
  k_fraction_change_sign(&b_negative);
  return k_fraction_add(a, &b_negative);
}

k_fraction k_fraction_multiply(k_fraction *a, k_fraction *b) {
  k_fraction a_cpy = *a;
  k_fraction b_cpy = *b;
  long num_mul = 0, denom_mul = 0;
  while (__builtin_mul_overflow(a_cpy.numerator, b_cpy.numerator, &num_mul) ||
         __builtin_mul_overflow(a_cpy.denominator, b_cpy.denominator, &denom_mul)) {
    if (!k_fraction_downcast_precision(&a_cpy) && !k_fraction_downcast_precision(&b_cpy)) {
      kpanicln("Can't avoid overflow when computing %i/%i * %i/%i.", a_cpy.numerator, a_cpy.denominator, b_cpy.numerator, b_cpy.denominator);
    }
  }

  k_fraction ret = k_fraction_create(num_mul, denom_mul);
  kdbgln("%i/%i * %i/%i = %i/%i", a->numerator, a->denominator, b->numerator, b->denominator, ret.numerator, ret.denominator);

  return ret;
}

int k_fraction_scalar_multiply(k_fraction *a, long scalar, k_fraction *out) {
  *out = *a;

  if (scalar == 0) {
    out->numerator = 0;
    return 0;
  }

  long num_mul = 0, denom_mul = 0;
  if (__builtin_mul_overflow(out->numerator, scalar, &num_mul) ||
      __builtin_mul_overflow(out->denominator, scalar, &denom_mul)) {
    return -K_ERANGE;
  }

  out->numerator = num_mul;
  out->denominator = denom_mul;
  kdbgln("%i/%i scaled by %i = %i/%i", a->numerator, a->denominator, scalar, out->numerator, out->denominator);

  return 0;
}

k_fraction k_fraction_floor(k_fraction *a) {
  k_fraction ret = k_fraction_create(a->numerator - a->numerator % a->denominator, a->denominator);
  kdbgln("Flooring %i/%i -> %i/%i", a->numerator, a->denominator, ret.numerator, ret.denominator);
  return ret;
}

k_fraction k_fraction_round(k_fraction *a) {
  k_fraction half = k_fraction_create(1, 2);
  k_fraction added = k_fraction_add(a, &half);
  k_fraction ret = k_fraction_floor(&added);
  kdbgln("Rounding %i/%i -> %i/%i", a->numerator, a->denominator, ret.numerator, ret.denominator);
  return ret;
}

k_fraction k_fraction_project_onto_range(k_fraction *a, k_fraction *min, k_fraction *max) {
  VERIFY(a->numerator <= a->denominator);
  VERIFY(!k_fraction_lt(max, min));

  k_fraction diff = k_fraction_sub(max, min);
  k_fraction scaled = k_fraction_multiply(a, &diff);
  k_fraction ret = k_fraction_add(&scaled, min);
  kdbgln("Projecting %i/%i onto [%i/%i, %i/%i] -> %i/%i", a->numerator, a->denominator, min->numerator, min->denominator, max->numerator, max->denominator, ret.numerator, ret.denominator);
  return ret;
}

long k_project_long_onto_range(long a, long min, long max) {
  k_fraction f_scale = k_fraction_create(a, LONG_MAX);
  k_fraction f_min = k_fraction_from_number(min);
  k_fraction f_max = k_fraction_from_number(max);
  k_fraction f_projected = k_fraction_project_onto_range(&f_scale, &f_min, &f_max);
  k_fraction f_rounded = k_fraction_round(&f_projected);
  return k_fraction_to_whole_number(&f_rounded);
}