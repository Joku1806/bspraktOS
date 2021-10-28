#include <kernel/drivers/pl001.h>
#include <kernel/kprintf.h>
#include <stdarg.h>
#include <stdint.h>

// in Teilen inspiriert von
// https://github.com/mpaland/printf/blob/master/printf.c und NICHT von der libc
// printf, wie im Aufgabenblatt empfohlen. Wenn wir das gemacht hätten, würde es
// wahrscheinlich Abzugpunkte bis in den Minusbereich hageln :^)

// interne Funktionen
int is_ascii_decimal_digit(char ch);
uint8_t ascii_to_decimal_digit(char in);
char decimal_digit_to_ascii(uint8_t in);
char hexadecimal_digit_to_ascii(uint8_t in);
void output_padding(kprintf_state *state);
void output_literal_percent();
void output_character(char ch);
void output_string(char *str);
void output_as_hexadecimal_number(uint32_t num, kprintf_state *state);
void output_as_decimal_number(uint32_t num, kprintf_state *state);
void output_as_signed_decimal_number(int32_t num, kprintf_state *state);
int handle_format_specifier(kprintf_state *state);
void set_flags(kprintf_state *state);
void set_pad_width(kprintf_state *state);
void kprintf_initialize_state(kprintf_state *state, const char *format);
void kprintf_reset_state(kprintf_state *state);

// FIXME: should be in a separate file
// Prüft, ob ch eine dezimale Ziffer ist
int is_ascii_decimal_digit(char ch) { return ch >= '0' && ch <= '9'; }

// Konviert die ASCII-Repräsentation einer Ziffer zu der eigentlichen Ziffer. Es
// wird angenommen, dass bei unbekanntem Input vorher is_ascii_decimal_digit()
// aufgerufen wurde.
uint8_t ascii_to_decimal_digit(char in) { return in - 30; }

// Konviert eine dezimale Ziffer zu der korrespondierenden ASCII-Repräsentation.
// Gibt -EINVAL zurück, falls in keine dezimale Ziffer ist.
char decimal_digit_to_ascii(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  }

  kprintf("%c is not a decimal digit.\n", in); // Rekursion!
  return -EINVAL;
}

// Konviert eine hexadezimale Ziffer zu der korrespondierenden
// ASCII-Repräsentation. Gibt -EINVAL zurück, falls in keine hexadezimale Ziffer
// ist.
char hexadecimal_digit_to_ascii(uint8_t in) {
  if (in <= 9) {
    return in + 30;
  } else if (in <= 15) {
    return in + 41;
  }

  kprintf("%c is not a hexadecimal digit.\n", in);
  return -EINVAL;
}

// Gibt state->pad_width viele Padding-Buchstaben aus. Wenn die zero_pad Flag
// gesetzt ist, ist dieser Buchstabe '0', ansonsten ' '.
void output_padding(kprintf_state *state) {
  char padding = state->flags & zero_pad ? '0' : ' ';
  while (state->pad_width) {
    pl001_send(padding);
    state->pad_width--;
  }
}

// Verantwortlich für %% und gibt '%' aus
void output_literal_percent() { pl001_send('%'); }

// Verantwortlich für %c und gibt kprintf Argument als Buchstabe aus.
void output_character(char ch) { pl001_send(ch); }

// Verantwortlich für %s und gibt kprintf Argument als String aus.
void output_string(char *str) {
  while (*str != '\0') {
    pl001_send(*str);
    str++;
  }
}

// Verantwortlich für %x/p und gibt kprintf Argument formatiert als hexadezimale
// Zahl aus.
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

// Verantwortlich für %u/i und gibt kprintf Argument formatiert als dezimale
// Zahl aus.
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

void output_as_signed_decimal_number(int32_t num, kprintf_state *state) {
  uint8_t num_width = DECIMAL_MAX_PRINT_WIDTH;
  uint32_t decimal_digit_checker = 1000000000;
  uint32_t unum = num & (1 << 31) ? -num : num;
  // find first decimal digit in num
  while (decimal_digit_checker > unum) {
    decimal_digit_checker /= 10;
    SAFE_DECREMENT(num_width, 1);
  }

  if (num & (1 << 31)) {
    if (state->flags & zero_pad) {
      pl001_send('-');
      SAFE_DECREMENT(state->pad_width, 1);
      output_padding(state);
    } else {
      SAFE_DECREMENT(state->pad_width, 1);
      output_padding(state);
      pl001_send('-');
    }
  } else {
    SAFE_DECREMENT(state->pad_width, num_width);
    output_padding(state);
  }

  while (decimal_digit_checker > 0) {
    uint8_t digit = unum / decimal_digit_checker;
    pl001_send(decimal_digit_to_ascii(digit));
    unum -= digit * decimal_digit_checker;
    decimal_digit_checker /= 10;
  }
}

// Wirkt als Dispatch-Funktion für die verschiedenen Format-Typen und prüft
// außerdem, ob die gewählten Flags kompatibel mit dem Format-Specifier
// sind. Wenn das nicht so ist, wird -EINVAL zurückgegeben. Es wird auch
// -EINVAL zurückgegeben, falls *state->position kein valider
// Format-Specifier ist.
int handle_format_specifier(kprintf_state *state) {
  if (*state->position == '\0') {
    return -EINVAL;
  }

  if (!(*state->position == 'i' || *state->position == 'u' ||
        *state->position == 'x' || *state->position == 'p') &&
      state->pad_width) {
    kprintf("Width field can't be used with format specifier %%%c.\n",
            *state->position);
    return -EINVAL;
  }

  if (*state->position == 'p' && state->flags & zero_pad) {
    kprintf("Zero-padding can't be used with format specifier %%%c.\n",
            *state->position);
    return -EINVAL;
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
    output_as_signed_decimal_number(num, state);
  } else {
    return -EINVAL;
  }

  kprintf_reset_state(state);
  // FIXME: return number of characters written
  return 0;
}

// Setzt alle angegebenen Flags, die unterstützt sind. Im Moment ist die einzige
// unterstützte Flag '0' :D
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

// Setzt die Feldbreite, wenn angegeben. Die Feldbreite muss als Dezimalzahl
// angegeben sein.
void set_pad_width(kprintf_state *state) {
  // FIXME: Doesn't check for overflow, especially important since
  // state->pad_width is an uint8_t
  while (is_ascii_decimal_digit(*state->position)) {
    state->pad_width *= 10;
    state->pad_width += ascii_to_decimal_digit(*state->position);
    state->position++;
  }
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
int kprintf(const char *format, ...) {
  kprintf_state state;
  va_start(state.arguments, format);
  kprintf_initialize_state(&state, format);

  do {
    if (*state.position == '%') {
      state.position++;
      set_flags(&state);
      set_pad_width(&state);
      int ret = handle_format_specifier(&state);
      if (ret < 0) {
        return ret;
      }
    } else {
      pl001_send(*state.position);
    }

    state.position++;
  } while (*state.position != '\0');

  va_end(state.arguments);
  return 0;
}