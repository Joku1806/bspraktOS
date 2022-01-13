#define LOG_LABEL "Math (Kernel)"
#define LOG_LEVEL WARNING_LEVEL

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kmath.h>
#include <stddef.h>

unsigned k_modulo_add(unsigned a, unsigned b, unsigned min, unsigned max) {
  VERIFY(min < max);
  a %= max;
  b %= max;

  if (a < max - b) {
    kdbgln("k_modulo_add(%u, %u, %u, %u) = %u", a, b, min, max, a + b);
    return a + b;
  }

  kdbgln("k_modulo_add(%u, %u, %u, %u) = %u", a, b, min, max, (a + b) - max + min);
  return (a + b) - max + min;
}

unsigned k_modulo_sub(unsigned a, unsigned b, unsigned min, unsigned max) {
  VERIFY(min < max);
  a %= max;
  b %= max;

  // FIXME: bin mir überhaupt nicht sicher ob das richtig ist lol
  if (K_MAX(a, min) - K_MIN(a, min) < b) {
    kdbgln("k_modulo_sub(%u, %u, %u, %u) = %u", a, b, min, max, max - (b - (K_MAX(a, min) - K_MIN(a, min))));
    return max - (b - (K_MAX(a, min) - K_MIN(a, min)));
  }

  kdbgln("k_modulo_sub(%u, %u, %u, %u) = %u", a, b, min, max, a - b);
  return a - b;
}

long k_gcd(long a, long b) {
  a = K_ABS(a);
  b = K_ABS(b);

  while (b) {
    long tmp = a;
    a = b;
    b = tmp % b;
  }

  return a;
}

long k_lcm(long a, long b) {
  return K_ABS(a) / k_gcd(a, b) * K_ABS(b);
}

unsigned k_clamp_unsigned(unsigned num, unsigned min, unsigned max) {
  VERIFY(min < max);
  return num < min ? min : (num > max ? max : num);
}

unsigned k_align8(unsigned num) {
  VERIFY(UINT_MAX - 8 >= num);
  return num % 8 != 0 ? num + 8 - (num % 8) : num;
}