#include <stddef.h>
#include <user/lib/assertions.h>
#include <user/lib/math.h>

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

unsigned align8(unsigned num) {
  VERIFY(UINT_MAX - 8 >= num);
  return num % 8 != 0 ? num + 8 - (num % 8) : num;
}