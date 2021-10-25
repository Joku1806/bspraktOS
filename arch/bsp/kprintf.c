#include "../../include/arch/bsp/kprintf.h"
#include <stdarg.h>
#include <stdint.h>

void PL001_UART_send(char ch) {
  // TODO: Everything :^)
}

char decimal_digit_to_char(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  }

  // TODO: error handling
}

char hexdigit_to_char(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  } else if (in <= 15) {
    return in + 41;
  }

  // TODO: error handling
}

void handle_format_specifier(const char *position, va_list *arguments) {
  // FIXME: Doesn't check if there is an argument left
  char following = *(position + 1);
  if (following == '\0') {
    // TODO: error handling
    return;
  }

  if (following == '%') {
    PL001_UART_send('%');
  } else if (following == 'c') {
    // FIXME: undefined behaviour no no
    char ch = va_arg(*arguments, int);
    PL001_UART_send(ch);
  } else if (following == 's') {
    char *string = va_arg(*arguments, char *);
    while (*string != '\0') {
      PL001_UART_send(*string);
      string++;
    }
  } else if (following == 'x' || following == 'p') {
    // extra cool prefix for addresses hell yeah
    if (following == 'p') {
      PL001_UART_send('0');
      PL001_UART_send('x');
    }

    // FIXME: How can we check that caller isn't passing a smaller data type
    // than uint32_t as an argument, fucking up every other format after %x?
    uint32_t num = va_arg(*arguments, uint32_t);
    while (num != 0) {
      uint8_t digit = num & 0xF0000000;
      PL001_UART_send(hexdigit_to_char(digit));
      num <<= 4;
    }
  } else if (following == 'u' || following == 'i') {
    uint32_t num = va_arg(*arguments, uint32_t);

    // if sign bit is not set, just treat it as an unsigned number
    if (following == 'i' && num & (1 << 31)) {
      PL001_UART_send('-');
      // is supposed to calculate 0b0x0000... - 0b00xxxx... to switch 2s
      // complement to unsigned, but I don't know if that even works how I think
      // it does, just leave it like this for now
      num = (num & (1 << 30)) - (num & ~(3 << 30));
    }

    uint32_t decimal_digit_checker = 1000000000;
    // find first decimal digit in num
    while (decimal_digit_checker > num) {
      decimal_digit_checker /= 10;
    }

    while (decimal_digit_checker > 0) {
      // should fit into uint8_t I hope
      uint8_t digit = num / decimal_digit_checker;
      PL001_UART_send(decimal_digit_to_char(digit));
      num -= digit * decimal_digit_checker;
      decimal_digit_checker /= 10;
    }
  } else {
    // TODO: you guessed it... error handling
    return;
  }
}

int kprintf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const char *position = format;

  do {
    if (*position == '%') {
      handle_format_specifier(position, &arguments);
      position++;
    } else {
      PL001_UART_send(*position);
    }

    position++;
  } while (*position != '\0');

  va_end(arguments);
  // should return number of characters written, but who cares right? ;)
  return 0;
}