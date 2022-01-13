#ifndef KMATH_H
#define KMATH_H

#define K_MIN(a, b) ((a) < (b) ? (a) : (b))
#define K_MAX(a, b) ((a) < (b) ? (b) : (a))
#define K_ABS(a) ((a) >= 0 ? (a) : -(a))

#define K_IS_POWER_OF_TWO(x) (((x) & ((x)-1)) == 0)

unsigned k_modulo_add(unsigned a, unsigned b, unsigned min, unsigned max);
unsigned k_modulo_sub(unsigned a, unsigned b, unsigned min, unsigned max);
long k_gcd(long a, long b);
long k_lcm(long a, long b);
unsigned k_clamp_unsigned(unsigned num, unsigned min, unsigned max);
unsigned k_align8(unsigned num);

#endif