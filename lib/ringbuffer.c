#include <lib/assertions.h>
#include <lib/debug.h>
#include <lib/ringbuffer.h>
#include <stddef.h>

#define LOGLEVEL WARNING_LEVEL

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

size_t last_read_index(ringbuffer *r) {
  return r->read_index == 0 ? r->length - 1 : r->read_index - 1;
}


char ringbuffer_read(ringbuffer *r) {
  VERIFY(r->read_index < r->length);
  r->ignore_writes = false;

  char ch = r->contents[r->read_index];

  if (r->read_index == r->write_index) {
    r->read_index = last_read_index(r);
  }

  r->read_index = (r->read_index + 1) % r->length;

  return ch;
}

void ringbuffer_write(ringbuffer *r, char ch) {
  VERIFY(r->write_index < r->length);

  r->valid_reads = true;

  if (r->ignore_writes) {
    return;
  }

  if (r->write_index == last_read_index(r)) {
    r->ignore_writes = true;
  }

  r->contents[r->write_index] = ch;
  r->write_index = (r->write_index + 1) % r->length;
}