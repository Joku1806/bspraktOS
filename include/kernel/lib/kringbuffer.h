#ifndef KRINGBUFFER_H
#define KRINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void *data;
  size_t mask;
  size_t read_index;
  size_t write_index;
} k_ringbuffer;

void k_ringbuffer_initialise(k_ringbuffer *r, void *data, size_t length);

bool k_ringbuffer_empty(k_ringbuffer *r);
bool k_ringbuffer_full(k_ringbuffer *r);

size_t k_ringbuffer_read(k_ringbuffer *r, void *dst, size_t length);
size_t k_ringbuffer_write(k_ringbuffer *r, const void *src, size_t length);

#endif