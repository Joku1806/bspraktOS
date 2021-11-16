#include <arch/bsp/pl001.h>
#include <kernel/debug.h>
#include <kernel/kprintf.h>
#include <lib/character_types.h>
#include <lib/error_codes.h>
#include <stdbool.h>
#include <stddef.h>
// in Teilen inspiriert von
// https://github.com/mpaland/printf/blob/master/printf.c und NICHT von der libc
// printf, wie im Aufgabenblatt empfohlen. Wenn wir das gemacht hätten, würde es
// wahrscheinlich Abzugpunkte bis in den Minusbereich hageln :^)

// interne Funktionen
size_t output_literal_percent();
size_t output_character(char ch);
size_t output_string(char *str);
size_t base_less_eq_16_to_ascii(uint32_t num, uint8_t base, char *out,
                                size_t max_length);
size_t format_and_output_number(unsigned long num, uint8_t base,
                                bool is_negative, kprintf_state *state);
int handle_format_specifier(kprintf_state *state);

void set_flags(kprintf_state *state);
int set_pad_width(kprintf_state *state);

void kprintf_initialize_state(kprintf_state *state, const char *format);
void kprintf_reset_state(kprintf_state *state);

// Verantwortlich für %% und gibt '%' aus
size_t output_literal_percent() {
  pl001_send('%');
  return sizeof(char);
}

// Verantwortlich für %c und gibt kprintf Argument als Buchstabe aus.
size_t output_character(char ch) {
  pl001_send(ch);
  return sizeof(char);
}

// Verantwortlich für %s und gibt kprintf Argument als String aus.
size_t output_string(char *str) {
  size_t chars_written = 0;
  while (*str != '\0') {
    pl001_send(*str);
    chars_written++;
    str++;
  }

  return chars_written;
}

// Konvertiert eine Zahl beliebiger Basis <= 16 zu
// der ASCII-Darstellung dieser Zahl. Die Darstellung
// ist aufgrund der Umwandlungsmethode umgekehrt, d.h.
// 12345 => "54321".
size_t base_less_eq_16_to_ascii(uint32_t num, uint8_t base, char *out,
                                size_t max_length) {
  size_t digits = 0;

  do {
    uint8_t digit = num % base;
    // supports only up to base 16
    out[digits++] = to_ascii_hexadecimal_digit(digit);
    num /= base;
  } while (num && (digits < max_length));

  return digits;
}

// Konvertiert num in ASCII-Darstellung und gibt diese mit optionaler
// Feldbreite (zusammen maximal MAX_NUMBER_PRINT_WIDTH Zeichen) formatiert aus.
size_t format_and_output_number(unsigned long num, uint8_t base,
                                bool is_negative, kprintf_state *state) {
  if (base > 16) {
    warnln("Base %u is greater than the allowed maximum of 16.", base);
    return -EINVAL;
  }

  char buffer[MAX_NUMBER_PRINT_WIDTH];
  size_t length =
      base_less_eq_16_to_ascii(num, base, buffer, MAX_NUMBER_PRINT_WIDTH);

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
    pl001_send(buffer[--length]);
  }

  return length_cpy;
}

// Wirkt als Dispatch-Funktion für die verschiedenen Format-Typen und prüft
// außerdem, ob die gewählten Flags kompatibel mit dem Format-Specifier
// sind. Wenn das nicht so ist, wird -EINVAL zurückgegeben. Es wird auch
// -EINVAL zurückgegeben, falls *state->position kein valider
// Format-Specifier ist.
int handle_format_specifier(kprintf_state *state) {
  if (*state->position == '\0') {
    warnln("Dangling %% is not allowed.");
    return -EINVAL;
  }

  if (!(*state->position == 'i' || *state->position == 'u' ||
        *state->position == 'x' || *state->position == 'p') &&
      state->pad_width) {
    warnln("Field width can't be used with format specifier %%%c.",
           *state->position);
    return -EINVAL;
  }

  if (*state->position == 'p' && state->flags & flag_zeropad) {
    warnln("Zero-padding can't be used with format specifier %%p.");
    return -EINVAL;
  }

  size_t chars_written;
  if (*state->position == '%') {
    chars_written = output_literal_percent();
  } else if (*state->position == 'c') {
    char ch = va_arg(state->arguments, int);
    chars_written = output_character(ch);
  } else if (*state->position == 's') {
    char *str = va_arg(state->arguments, char *);
    chars_written = output_string(str);
  } else if (*state->position == 'x') {
    unsigned int hex = va_arg(state->arguments, unsigned int);
    chars_written = format_and_output_number(hex, 16, false, state);
  } else if (*state->position == 'p') {
    state->flags |= flag_hash;
    void *ptr = va_arg(state->arguments, void *);
    chars_written =
        format_and_output_number((unsigned long)ptr, 16, false, state);
  } else if (*state->position == 'u') {
    unsigned int num = va_arg(state->arguments, unsigned int);
    chars_written = format_and_output_number(num, 10, false, state);
  } else if (*state->position == 'i') {
    int num = va_arg(state->arguments, int);
    bool is_negative = num < 0;
    if (is_negative) {
      num = -num;
    }

    chars_written = format_and_output_number(num, 10, is_negative, state);
  } else {
    warnln("kprintf doesn't support format specifier %%%c.", *state->position);
    return -EINVAL;
  }

  // durch cast von size_t -> int gehen möglicherweise Informationen verloren,
  // obwohl das schon sehr unwahrscheinlich ist
  return chars_written;
}

// Setzt alle angegebenen Flags, die unterstützt sind. Im Moment ist die einzige
// unterstützte Flag '0' :D. Es gibt auch noch flag_hash, aber die wird im
// Moment nur dazu benutzt, um intern 0x bei %p anzuhängen.
void set_flags(kprintf_state *state) {
  int flags_finished = 0;
  while (!flags_finished) {
    switch (*state->position) {
    case '0':
      state->flags |= flag_zeropad;
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
int set_pad_width(kprintf_state *state) {
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
void kprintf_initialize_state(kprintf_state *state, const char *format) {
  state->position = format;
  state->flags = 0;
  state->pad_width = 0;
}

// Wird dazu benutzt, um den Zustand vor einem Format Specifier
// wiederherzustellen.
void kprintf_reset_state(kprintf_state *state) {
  state->flags = 0;
  state->pad_width = 0;
}

// Kernelkompatible printf-Funktion
// Diese Funktion kann dafür verwendet werden, Debug-Output zu generieren.
// Von den normalen printf-Features werden %c/s/%/x/p/u/i sowie die zero_pad
// flag und variable Feldbreite unterstützt. Gibt -EINVAL zurück, falls es einen
// Syntaxfehler im angegebenen Format gibt.
__attribute__((format(printf, 1, 2))) int kprintf(const char *format, ...) {
  int chars_written = 0;
  int ret;
  kprintf_state state;
  va_start(state.arguments, format);
  kprintf_initialize_state(&state, format);

  while (*state.position != '\0') {
    if (*state.position == '%') {
      state.position++;
      set_flags(&state);
      ret = set_pad_width(&state);
      if (ret < 0) {
        va_end(state.arguments);
        return ret;
      }

      ret = handle_format_specifier(&state);
      if (ret < 0) {
        va_end(state.arguments);
        return ret;
      }

      chars_written += ret;
      kprintf_reset_state(&state);
    } else {
      pl001_send(*state.position);
      chars_written++;
    }

    state.position++;
  }

  va_end(state.arguments);
  return chars_written;
}