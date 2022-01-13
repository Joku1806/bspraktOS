// in Teilen inspiriert von
// https://github.com/mpaland/printf/blob/master/printf.c und NICHT von der
// libc printf, wie im Aufgabenblatt empfohlen. Wenn wir das gemacht hätten,
// würde es wahrscheinlich Abzugpunkte bis in den Minusbereich hageln :^)

#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "printf"

#include <stdbool.h>
#include <stddef.h>
#include <user/lib/ascii.h>
#include <user/lib/assertions.h>
#include <user/lib/debug.h>
#include <user/lib/error.h>
#include <user/lib/printf.h>
#include <user/lib/syscall.h>

// interne Funktionen
size_t printf_output_literal_percent();
size_t printf_output_character(char ch);
size_t printf_output_string(char *str, printf_state *state);
size_t printf_base_less_eq_16_to_ascii(uint32_t num, uint8_t base, char *out,
                                       size_t max_length);
size_t printf_format_and_output_number(unsigned long num, uint8_t base,
                                       bool is_negative, printf_state *state);
int printf_handle_format_specifier(printf_state *state);

void printf_set_flags(printf_state *state);
int printf_set_pad_width(printf_state *state);

void printf_initialize_state(printf_state *state, const char *format);
void printf_reset_state(printf_state *state);

// Verantwortlich für %% und gibt '%' aus
size_t printf_output_literal_percent() {
  sys$output_character('%');
  return sizeof(char);
}

// Verantwortlich für %c und gibt printf Argument als Buchstabe aus.
size_t printf_output_character(char ch) {
  sys$output_character(ch);
  return sizeof(char);
}

// Verantwortlich für %s und gibt printf Argument als String aus.
size_t printf_output_string(char *str, printf_state *state) {
  size_t chars_written = 0;
  while (*str != '\0') {
    sys$output_character(*str);
    chars_written++;
    str++;
  }

  while (chars_written < state->pad_width) {
    sys$output_character(' ');
    chars_written++;
  }

  return chars_written;
}

// Konvertiert eine Zahl beliebiger Basis <= 16 zu
// der ASCII-Darstellung dieser Zahl. Die Darstellung
// ist aufgrund der Umwandlungsmethode umgekehrt, d.h.
// 12345 => "54321".
size_t printf_base_less_eq_16_to_ascii(uint32_t num, uint8_t base, char *out,
                                       size_t max_length) {
  VERIFY(base <= 16);
  size_t digits = 0;

  do {
    uint8_t digit = num % base;
    out[digits++] = to_ascii_hexadecimal_digit(digit);
    num /= base;
  } while (num && (digits < max_length));

  return digits;
}

// Konvertiert num in ASCII-Darstellung und gibt diese mit optionaler
// Feldbreite (zusammen maximal MAX_NUMBER_PRINT_WIDTH Zeichen) formatiert aus.
size_t printf_format_and_output_number(unsigned long num, uint8_t base,
                                       bool is_negative, printf_state *state) {
  if (base > 16) {
    warnln("Base %u is greater than the allowed maximum of 16.", base);
    return -EINVAL;
  }

  char buffer[MAX_NUMBER_PRINT_WIDTH];
  size_t length =
      printf_base_less_eq_16_to_ascii(num, base, buffer, MAX_NUMBER_PRINT_WIDTH);

  if (state->pad_width > MAX_NUMBER_PRINT_WIDTH) {
    state->pad_width = MAX_NUMBER_PRINT_WIDTH;
  }

  if (state->flags & flag_zeropad) {
    if (state->flags & flag_hash) {
      SAFE_DECREMENT(state->pad_width, 2);
    }

    if (is_negative) {
      SAFE_DECREMENT(state->pad_width, 1);
    }

    while (length < state->pad_width) {
      buffer[length++] = '0';
    }
  }

  if (state->flags & flag_hash) {
    if (base == 16) {
      buffer[length++] = 'x';
    } else if (base == 2) {
      buffer[length++] = 'b';
    }
    buffer[length++] = '0';
  }

  if (is_negative) {
    buffer[length++] = '-';
  }

  while (length < state->pad_width) {
    buffer[length++] = ' ';
  }

  size_t length_cpy = length;
  while (length) {
    sys$output_character(buffer[--length]);
  }

  return length_cpy;
}

// Wirkt als Dispatch-Funktion für die verschiedenen Format-Typen und prüft
// außerdem, ob die gewählten Flags kompatibel mit dem Format-Specifier
// sind. Wenn das nicht so ist, wird -EINVAL zurückgegeben. Es wird auch
// -EINVAL zurückgegeben, falls *state->position kein valider
// Format-Specifier ist.
int printf_handle_format_specifier(printf_state *state) {
  if (*state->position == '\0') {
    warnln("Dangling %% is not allowed.");
    return -EINVAL;
  }

  if (*state->position == 'p' && state->flags & flag_zeropad) {
    warnln("Zero-padding can't be used with format specifier %%p.");
    return -EINVAL;
  }

  int chars_written;
  switch (*state->position) {
    case '%':
      chars_written = printf_output_literal_percent();
      break;

    case 'c':
      chars_written = printf_output_character(va_arg(state->arguments, int));
      break;

    case 's':
      chars_written = printf_output_string(va_arg(state->arguments, char *), state);
      break;

    case 'b':
      chars_written = printf_format_and_output_number(
          va_arg(state->arguments, unsigned int), 2, false, state);
      break;

    case 'x':
      chars_written = printf_format_and_output_number(
          va_arg(state->arguments, unsigned int), 16, false, state);
      break;

    case 'p':
      state->flags |= flag_hash;
      chars_written = printf_format_and_output_number(
          (unsigned long)va_arg(state->arguments, void *), 16, false, state);
      break;

    case 'u':
      chars_written = printf_format_and_output_number(
          va_arg(state->arguments, unsigned int), 10, false, state);
      break;

    case 'i': {
      int num = va_arg(state->arguments, int);
      bool is_negative = num < 0;
      if (is_negative) {
        num = -num;
      }

      chars_written = printf_format_and_output_number(num, 10, is_negative, state);
      break;
    }

    default:
      warnln("printf doesn't support format specifier %%%c.",
             *state->position);
      return -EINVAL;
  }

  return chars_written;
}

// Setzt alle angegebenen Flags, die unterstützt sind.
// Im Moment sind das Zeropad und Hash.
void printf_set_flags(printf_state *state) {
  int flags_finished = 0;
  while (!flags_finished) {
    switch (*state->position) {
      case '0':
        state->flags |= flag_zeropad;
        state->position++;
        break;
      case '#':
        state->flags |= flag_hash;
        state->position++;
        break;
      default:
        flags_finished = 1;
    }
  }
}

// Setzt die Feldbreite, wenn angegeben. Die Feldbreite muss als Dezimalzahl
// angegeben sein und in einen uint8_t passen. Sollte das nicht so sein,
// wird -EINVAL zurückgegeben.
int printf_set_pad_width(printf_state *state) {
  uint8_t converted = 0;
  while (is_ascii_decimal_digit(*state->position)) {
    uint8_t digit = parse_ascii_decimal_digit(*state->position);
    if (converted * 10 + digit > UINT8_MAX) {
      warnln("Specified field width is too big to be stored in an uint8_t.");
      return -EINVAL;
    }

    converted *= 10;
    converted += digit;
    state->position++;
  }

  state->pad_width = converted;
  return 0;
}

// Initialisiert ein gemeinsames struct, um Informationen über die
// Ausgabeparameter an einem Ort zu sammeln. Das macht es einfacher, jeder
// Funktion alle nötigen Informationen zur Verfügung zu stellen, da dafür nur
// ein Argument gebraucht wird.
void printf_initialize_state(printf_state *state, const char *format) {
  state->position = format;
  state->flags = 0;
  state->pad_width = 0;
}

// Wird dazu benutzt, um den Zustand vor einem Format Specifier
// wiederherzustellen.
void printf_reset_state(printf_state *state) {
  state->flags = 0;
  state->pad_width = 0;
}

// user printf-Funktion
// Diese Funktion kann dafür verwendet werden, Debug-Output zu generieren.
// Von den normalen printf-Features werden %c/s/%/x/p/u/i sowie die zero_pad
// flag und variable Feldbreite unterstützt. Gibt -EINVAL zurück, falls es
// einen Syntaxfehler im angegebenen Format gibt.
__attribute__((format(printf, 1, 2))) int printf(const char *format, ...) {
  int chars_written = 0;
  int ret;
  printf_state state;
  va_start(state.arguments, format);
  printf_initialize_state(&state, format);

  while (*state.position != '\0') {
    if (*state.position == '%') {
      state.position++;
      printf_set_flags(&state);
      ret = printf_set_pad_width(&state);
      if (ret < 0) {
        va_end(state.arguments);
        return ret;
      }

      ret = printf_handle_format_specifier(&state);
      if (ret < 0) {
        va_end(state.arguments);
        return ret;
      }

      chars_written += ret;
      printf_reset_state(&state);
    } else {
      sys$output_character(*state.position);
      chars_written++;
    }

    state.position++;
  }

  va_end(state.arguments);
  return chars_written;
}