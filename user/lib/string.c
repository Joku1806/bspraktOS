#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "String"

#include <user/lib/assertions.h>
#include <user/lib/string.h>

size_t strlen(const char *s) {
  VERIFY(s != NULL);
  size_t length = 0;
  while (*s++ != '\0') {
    length++;
  }
  return length;
}

void *memcpy(void *dst, const void *src, size_t n) {
  VERIFY(dst != NULL);
  VERIFY(src != NULL);

  char *dst_c = (char *)dst;
  char *src_c = (char *)src;

  for (size_t i = 0; i < n; i++) {
    dst_c[i] = src_c[i];
  }

  return dst;
}