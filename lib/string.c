#include "string.h"

size_t strlen(const char *s) {
  size_t length = 0;
  while (*s++ != '\0') {
    length++;
  }
  return length;
}