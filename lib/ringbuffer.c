#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Ringbuffer"

#include <lib/assertions.h>
#include <lib/debug.h>
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

size_t last_read_index(ringbuffer *r) {
  return r->read_index == 0 ? r->length - 1 : r->read_index - 1;
}

char ringbuffer_read(ringbuffer *r) {
  VERIFY(r->read_index < r->length);
  r->ignore_writes = false;
  while (!r->valid_reads) {}

  dbgln("Read character code %u at read index %u (write index %u)", ch,
        r->read_index, r->write_index);

  char ch;
  if (r->read_index == r->write_index) {
    dbgln("Read index hit write index %u, now stalling until another write "
          "occurs.",
          r->write_index);
    r->read_index = last_read_index(r);
    ch = r->contents[r->read_index];
  } else {
    r->read_index = (r->read_index + 1) % r->length;
    ch = r->contents[r->read_index];
  }

  return ch;
}

void ringbuffer_write(ringbuffer *r, char ch) {
  VERIFY(r->write_index < r->length);
  r->valid_reads = true;

  if (r->ignore_writes) {
    return;
  }

  if (r->write_index == last_read_index(r)) {
    dbgln("Write index will hit read index %u in next call, now blocking until "
          "read is called again.",
          r->read_index);
    r->ignore_writes = true;
  }

  r->contents[r->write_index] = ch;
  r->write_index = (r->write_index + 1) % r->length;
}