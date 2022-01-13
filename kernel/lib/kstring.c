#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "String"

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kstring.h>

size_t k_strlen(const char *s) {
  VERIFY(s != NULL);
  size_t length = 0;
  while (*s++ != '\0') {
    length++;
  }
  return length;
}

void *k_memcpy(void *dst, const void *src, size_t n) {
  if (n != 0) {
    VERIFY(dst != NULL);
    VERIFY(src != NULL);
  }

  char *dst_c = (char *)dst;
  char *src_c = (char *)src;

  for (size_t i = 0; i < n; i++) {
    dst_c[i] = src_c[i];
  }

  return dst;
}