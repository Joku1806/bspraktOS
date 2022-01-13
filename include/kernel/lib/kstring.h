#ifndef KSTRING_H
#define KSTRING_H

#include <stddef.h>

size_t k_strlen(const char *s);
void *k_memcpy(void *dst, const void *src, size_t n);

#endif