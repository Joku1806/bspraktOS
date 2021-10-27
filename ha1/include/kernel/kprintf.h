#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stdint.h>

#define HEXADECIMAL_MAX_PRINT_WIDTH 8
#define DECIMAL_MAX_PRINT_WIDTH 10

#define SAFE_DECREMENT(a, b) a = a > b ? a - b : 0

typedef enum {
  EFORMAT,
  ESPECIFIER,
  EINVALIDFLAG,
} kprintf_error;

typedef enum {
  zero_pad = 1 << 0,
} format_flags;

typedef struct {
  va_list arguments;
  const char *position;
  format_flags flags;
  uint8_t pad_width;
  kprintf_error error;
} kprintf_state;

int kprintf(const char *format, ...);

#endif