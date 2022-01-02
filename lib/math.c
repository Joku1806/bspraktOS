#include <lib/assertions.h>
#include <lib/math.h>
#include <stddef.h>

long gcd(long a, long b) {
  a = ABS(a);
  b = ABS(b);

  while (b) {
    long tmp = a;
    a = b;
    b = tmp % b;
  }

  return a;
}

long lcm(long a, long b) {
  return ABS(a) / gcd(a, b) * ABS(b);
}

unsigned clamp_unsigned(unsigned num, unsigned min, unsigned max) {
  VERIFY(min < max);
  return num < min ? min : (num > max ? max : num);
}