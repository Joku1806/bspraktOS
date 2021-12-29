#include <lib/math.h>
#include <stddef.h>

unsigned long gcd(unsigned long a, unsigned long b) {
  while (b) {
    long tmp = a;
    a = b;
    b = tmp % b;
  }

  return a;
}

unsigned long lcm(unsigned long a, unsigned long b) {
  return ABS(a) / gcd(a, b) * ABS(b);
}