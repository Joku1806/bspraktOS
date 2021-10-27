#include <kernel/drivers/pl001.h>
#include <kernel/kprintf.h>
#include <stdarg.h>
#include <stdint.h>

// FIXME: should be in a separate file
int is_ascii_decimal_digit(char ch) { return ch >= '0' && ch <= '9'; }

uint8_t ascii_to_decimal_digit(char in) { return in - 30; }

char decimal_digit_to_ascii(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  }

  // TODO: error handling
}

char hexadecimal_digit_to_ascii(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  } else if (in <= 15) {
    return in + 41;
  }

  // TODO: error handling
}

void output_literal_percent() { pl001_send('%'); }

void output_character(char ch) { pl001_send(ch); }

void output_string(char *str) {
  while (*str != '\0') {
    pl001_send(*str);
    str++;
  }
}

void output_padding(kprintf_state *state) {
  char padding = state->flags & zero_pad ? '0' : ' ';
  while (state->pad_width) {
    pl001_send(padding);
    state->pad_width--;
  }
}

void output_as_hexadecimal_number(uint32_t num, kprintf_state *state) {
  uint8_t num_width = HEXADECIMAL_MAX_PRINT_WIDTH;
  // consume leading zeros
  while ((num & 0xF0000000) == 0) {
    num <<= 4;
    SAFE_DECREMENT(num_width, 1);
  }

  SAFE_DECREMENT(state->pad_width, num_width);
  output_padding(state);

  while (num != 0) {
    uint8_t digit = num & 0xF0000000;
    pl001_send(hexadecimal_digit_to_ascii(digit));
    num <<= 4;
  }
}

void output_as_decimal_number(uint32_t num, kprintf_state *state) {
  uint8_t num_width = DECIMAL_MAX_PRINT_WIDTH;
  uint32_t decimal_digit_checker = 1000000000;
  // find first decimal digit in num
  while (decimal_digit_checker > num) {
    decimal_digit_checker /= 10;
    SAFE_DECREMENT(num_width, 1);
  }

  SAFE_DECREMENT(state->pad_width, num_width);
  output_padding(state);

  while (decimal_digit_checker > 0) {
    uint8_t digit = num / decimal_digit_checker;
    pl001_send(decimal_digit_to_ascii(digit));
    num -= digit * decimal_digit_checker;
    decimal_digit_checker /= 10;
  }
}

void handle_format_specifier(kprintf_state *state) {
  if (*state->position == '\0') {
    state->error = -EFORMAT;
    return;
  }

  if (!(*state->position == 'i' || *state->position == 'u' ||
        *state->position == 'x' || *state->position == 'p')) {
    // TODO: throw error or nah?
    state->pad_width = 0;
  }

  if (*state->position == 'p') {
    state->flags &= ~(zero_pad);
  }

  if (*state->position == '%') {
    output_literal_percent();
  } else if (*state->position == 'c') {
    char ch = va_arg(state->arguments, int);
    output_character(ch);
  } else if (*state->position == 's') {
    char *str = va_arg(state->arguments, char *);
    output_string(str);
  } else if (*state->position == 'x') {
    uint32_t num = va_arg(state->arguments, uint32_t);
    output_as_hexadecimal_number(num, state);
  } else if (*state->position == 'p') {
    uint32_t num = va_arg(state->arguments, uint32_t);
    pl001_send('0');
    pl001_send('x');
    SAFE_DECREMENT(state->pad_width, 2);
    output_as_hexadecimal_number(num, state);
  } else if (*state->position == 'u') {
    uint32_t num = va_arg(state->arguments, uint32_t);
    output_as_decimal_number(num, state);
  } else if (*state->position == 'i') {
    int32_t num = va_arg(state->arguments, int32_t);

    if (num & (1 << 31)) {
      // FIXME: if flag_zero is set, zeros should come after '-',
      // but otherwise '-' should come after spaces.
      // This is not implemented yet because we would have to rewrite
      // output_as_decimal_number for this and I'm too lazy for that rn :^)
      pl001_send('-');
      num = -num;
      SAFE_DECREMENT(state->pad_width, 1);
    }

    output_as_decimal_number(num, state);
  } else {
    state->error = -EINVALIDFLAG;
    return;
  }
}

void set_flags(kprintf_state *state) {
  int flags_finished = 0;
  while (!flags_finished) {
    switch (*state->position) {
    case '0':
      state->flags |= zero_pad;
      state->position++;
      break;
    default:
      flags_finished = 1;
    }
  }
}

void set_pad_width(kprintf_state *state) {
  // FIXME: Doesn't check for overflow
  while (is_ascii_decimal_digit(*state->position)) {
    state->pad_width *= 10;
    state->pad_width += ascii_to_decimal_digit(*state->position);
    state->position++;
  }
}

void kprintf_initialize_state(kprintf_state *state, const char *format) {
  state->position = format;
  state->flags = 0;
  state->pad_width = 0;
  state->error = 0;
}

int kprintf(const char *format, ...) {
  kprintf_state state;
  va_start(state.arguments, format);
  kprintf_initialize_state(&state, format);

  do {
    if (*state.position == '%') {
      state.position++;
      set_flags(&state);
      set_pad_width(&state);
      handle_format_specifier(&state);
    } else {
      pl001_send(*state.position);
    }

    state.position++;
  } while (*state.position != '\0');

  va_end(state.arguments);
  // should return number of characters written, but who cares right? ;)
  return state.error != 0 ? state.error : 0;
}