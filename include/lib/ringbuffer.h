#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void *data;
  size_t mask;
  size_t read_index;
  size_t write_index;
} ringbuffer;

void ringbuffer_initialise(ringbuffer *r, void *data, size_t length);

bool ringbuffer_empty(ringbuffer *r);
bool ringbuffer_full(ringbuffer *r);

size_t ringbuffer_read(ringbuffer *r, void *dst, size_t length);
size_t ringbuffer_write(ringbuffer *r, const void *src, size_t length);

#endif