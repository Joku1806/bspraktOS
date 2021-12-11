#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Ringbuffer"

#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/modmath.h>
#include <lib/ringbuffer.h>
#include <stddef.h>

ringbuffer ringbuffer_create(char *contents, size_t length) {
  VERIFY(length != 0);

  ringbuffer new = {
      .contents = contents,
      .length = length,
      .read_index = 0,
      .write_index = 0,
      .ignore_writes = false,
      .valid_reads = false,
  };

  return new;
}

bool ringbuffer_write_occured(ringbuffer *r) {
  return MODULO_ADD(r->read_index, 1, r->length) != r->write_index;
}

char ringbuffer_read(ringbuffer *r) {
  VERIFY(r->read_index < r->length);
  r->ignore_writes = false;
  while (!r->valid_reads) {}

  if (MODULO_ADD(r->read_index, 1, r->length) != r->write_index) {
    r->read_index = MODULO_ADD(r->read_index, 1, r->length);
  }

  return r->contents[r->read_index];
}

void ringbuffer_write(ringbuffer *r, char ch) {
  VERIFY(r->write_index < r->length);
  r->valid_reads = true;

  if (r->ignore_writes) {
    return;
  }

  if (MODULO_ADD(r->write_index, 1, r->length) == r->read_index) {
    dbgln("Write index will hit read index %u in next call, now blocking until "
          "read is called again.",
          r->read_index);
    r->ignore_writes = true;
  }

  r->contents[r->write_index] = ch;
  r->write_index = MODULO_ADD(r->write_index, 1, r->length);
}