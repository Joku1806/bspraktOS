#include <kernel/kprintf.h>
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

char hexadecimal_digit_to_char(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  } else if (in <= 15) {
    return in + 41;
  }

  // TODO: error handling
}

void output_literal_percent() { PL001_UART_send('%'); }

void output_character(char ch) { PL001_UART_send(ch); }

void output_string(char *str) {
  while (*str != '\0') {
    PL001_UART_send(*str);
    str++;
  }
}

void output_as_hexadecimal_number(uint32_t num) {
  while (num != 0) {
    uint8_t digit = num & 0xF0000000;
    PL001_UART_send(hexadecimal_digit_to_char(digit));
    num <<= 4;
  }
}

void output_as_decimal_number(uint32_t num) {
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
}

void handle_format_specifier(const char *position, va_list *arguments) {
  char following = *(position + 1);
  if (following == '\0') {
    // TODO: error handling
    return;
  }

  if (following == '%') {
    output_literal_percent();
  } else if (following == 'c') {
    char ch = va_arg(*arguments, int);
    output_character(ch);
  } else if (following == 's') {
    char *str = va_arg(*arguments, char *);
    output_string(str);
  } else if (following == 'x') {
    uint32_t num = va_arg(*arguments, uint32_t);
    output_as_hexadecimal_number(num);
  } else if (following == 'p') {
    uint32_t num = va_arg(*arguments, uint32_t);
    PL001_UART_send('0');
    PL001_UART_send('x');
    output_as_hexadecimal_number(num);
  } else if (following == 'u') {
    uint32_t num = va_arg(*arguments, uint32_t);
    output_as_decimal_number(num);
  } else if (following == 'i') {
    int32_t num = va_arg(*arguments, int32_t);

    if (num & (1 << 31)) {
      PL001_UART_send('-');
      num = -num;
    }

    output_as_decimal_number(num);
  } else {
    // TODO: you guessed it... error handling
    return;
  }
}

int kprintf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  // TODO: check if we even need to make a copy
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