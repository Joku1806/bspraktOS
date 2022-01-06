#ifndef MATH_H
#define MATH_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#define MODULO_ADD(x, i, m) (((x) + (i)) % (m))
// nimmt an, dass x, i <= m sind
#define MODULO_SUB(x, i, m) ((x) >= (i) ? (x) - (i) : (m) - (i) + (x))

#define IS_POWER_OF_TWO(x) (((x) & ((x)-1)) == 0)

long gcd(long a, long b);
long lcm(long a, long b);
unsigned clamp_unsigned(unsigned num, unsigned min, unsigned max);
unsigned align8(unsigned num);

#endif