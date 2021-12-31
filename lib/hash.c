#include <lib/hash.h>
#include <lib/math.h>
#include <lib/string.h>
#include <stddef.h>

// inspiriert von https://stackoverflow.com/questions/8317508/hash-function-for-a-string
#define P1 54059
#define P2 76963

long hash_string(const char *str, long seed) {
  long h = seed;
  for (size_t i = 0; i < strlen(str); i++) {
    h = (h * P1) ^ (str[i] * P2);
  }

  return ABS(h);
}