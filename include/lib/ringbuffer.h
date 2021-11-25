#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
  char *contents;
  size_t length;
  size_t read_index;
  size_t write_index;
  bool ignore_writes;
  volatile bool valid_reads;
} ringbuffer;

ringbuffer ringbuffer_create(char *contents, size_t length);

void ringbuffer_write(ringbuffer *r, char ch);
char ringbuffer_read(ringbuffer *r);

#endif