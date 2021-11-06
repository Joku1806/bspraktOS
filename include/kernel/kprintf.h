#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stdint.h>

#define MAX_NUMBER_PRINT_WIDTH 32
#define SAFE_DECREMENT(a, b) a = a > b ? a - b : 0

// FIXME: should go in separate errno file
typedef enum {
  EINVAL = 1,
} kprintf_error;

typedef enum {
  flag_zeropad = 1 << 0,
  flag_hash = 1 << 1,
} format_flags;

typedef struct {
  va_list arguments;
  const char *position;
  format_flags flags;
  uint8_t pad_width;
} kprintf_state;

int kprintf(const char *format, ...);

#endif