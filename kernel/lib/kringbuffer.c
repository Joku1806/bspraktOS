// Inspiriert von https://github.com/torvalds/linux/blob/master/lib/kfifo.c

#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Ringbuffer"

#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kmath.h>
#include <kernel/lib/kringbuffer.h>
#include <kernel/lib/kstring.h>
#include <stddef.h>

size_t k_ringbuffer_used(k_ringbuffer *r);
size_t k_ringbuffer_unused(k_ringbuffer *r);
void k_ringbuffer_read_with_offset(k_ringbuffer *r, void *dst, size_t length, size_t offset);
void k_ringbuffer_write_with_offset(k_ringbuffer *r, const void *src, size_t length, size_t offset);

size_t k_ringbuffer_used(k_ringbuffer *r) {
  return r->write_index - r->read_index;
}

size_t k_ringbuffer_unused(k_ringbuffer *r) {
  return (r->mask + 1) - (r->write_index - r->read_index);
}

void k_ringbuffer_read_with_offset(k_ringbuffer *r, void *dst, size_t length, size_t offset) {
  size_t size = r->mask + 1;
  offset &= r->mask;
  size_t rest_length = K_MIN(length, size - offset);

  k_memcpy(dst, r->data + offset, rest_length);
  k_memcpy(dst + rest_length, r->data, length - rest_length);
}

void k_ringbuffer_write_with_offset(k_ringbuffer *r, const void *src, size_t length, size_t offset) {
  size_t size = r->mask + 1;
  offset &= r->mask;
  size_t rest_length = K_MIN(length, size - offset);

  k_memcpy(r->data + offset, src, rest_length);
  k_memcpy(r->data, src + rest_length, length - rest_length);
}

void k_ringbuffer_initialise(k_ringbuffer *r, void *data, size_t length) {
  VERIFY(data != NULL);
  VERIFY(length >= 2 && K_IS_POWER_OF_TWO(length));

  r->data = data;
  r->mask = length - 1;
  r->read_index = 0;
  r->write_index = 0;
}

bool k_ringbuffer_empty(k_ringbuffer *r) {
  return k_ringbuffer_used(r) == 0;
}

bool k_ringbuffer_full(k_ringbuffer *r) {
  return k_ringbuffer_unused(r) == 0;
}

size_t k_ringbuffer_read(k_ringbuffer *r, void *dst, size_t length) {
  size_t cap = k_ringbuffer_used(r);

  if (length > cap) {
    length = cap;
  }

  kdbgln("Reading %u characters from ringbuffer %p.", length, r);
  k_ringbuffer_read_with_offset(r, dst, length, r->read_index);
  r->read_index += length;
  return length;
}

size_t k_ringbuffer_write(k_ringbuffer *r, const void *src, size_t length) {
  size_t cap = k_ringbuffer_unused(r);

  if (length > cap) {
    length = cap;
  }

  kdbgln("Writing %u characters to ringbuffer %p.", length, r);
  k_ringbuffer_write_with_offset(r, src, length, r->write_index);
  r->write_index += length;
  return length;
}