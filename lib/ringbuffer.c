// Inspiriert von https://github.com/torvalds/linux/blob/master/lib/kfifo.c

#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Ringbuffer"

#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/math.h>
#include <lib/ringbuffer.h>
#include <stddef.h>
#include <string.h>

size_t ringbuffer_used(ringbuffer *r);
size_t ringbuffer_unused(ringbuffer *r);
void ringbuffer_read_with_offset(ringbuffer *r, void *dst, size_t length, size_t offset);
void ringbuffer_write_with_offset(ringbuffer *r, const void *src, size_t length, size_t offset);

size_t ringbuffer_used(ringbuffer *r) {
  return r->write_index - r->read_index;
}

size_t ringbuffer_unused(ringbuffer *r) {
  return (r->mask + 1) - (r->write_index - r->read_index);
}

void ringbuffer_read_with_offset(ringbuffer *r, void *dst, size_t length, size_t offset) {
  size_t size = r->mask + 1;
  offset &= r->mask;
  size_t rest_length = MIN(length, size - offset);

  memcpy(dst, r->data + offset, rest_length);
  memcpy(dst + rest_length, r->data, length - rest_length);
}

void ringbuffer_write_with_offset(ringbuffer *r, const void *src, size_t length, size_t offset) {
  size_t size = r->mask + 1;
  offset &= r->mask;
  size_t rest_length = MIN(length, size - offset);

  memcpy(r->data + offset, src, rest_length);
  memcpy(r->data, src + rest_length, length - rest_length);
}

void ringbuffer_initialise(ringbuffer *r, void *data, size_t length) {
  VERIFY(data != NULL);
  VERIFY(length >= 2 && IS_POWER_OF_TWO(length));

  r->data = data;
  r->mask = length - 1;
  r->read_index = 0;
  r->write_index = 0;
}

bool ringbuffer_empty(ringbuffer *r) {
  return ringbuffer_used(r) == 0;
}

bool ringbuffer_full(ringbuffer *r) {
  return ringbuffer_unused(r) == 0;
}

size_t ringbuffer_read(ringbuffer *r, void *dst, size_t length) {
  size_t cap = ringbuffer_used(r);

  if (length > cap) {
    length = cap;
  }

  ringbuffer_read_with_offset(r, dst, length, r->read_index);
  r->read_index += length;
  return length;
}

size_t ringbuffer_write(ringbuffer *r, const void *src, size_t length) {
  size_t cap = ringbuffer_unused(r);

  if (length > cap) {
    length = cap;
  }

  ringbuffer_write_with_offset(r, src, length, r->write_index);
  r->write_index += length;
  return length;
}