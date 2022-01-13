#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "ASCII Conversion"

#include <kernel/lib/kascii.h>
#include <kernel/lib/kassertions.h>

bool k_is_ascii_decimal_digit(char ch) { return ch >= '0' && ch <= '9'; }

uint8_t k_parse_ascii_decimal_digit(char ch) {
  VERIFY(ch >= '0' && ch <= '9');
  return ch - '0';
}

char k_to_ascii_hexadecimal_digit(uint8_t in) {
  VERIFY(in <= 15);

  if (in <= 9) {
    return in + '0';
  } else {
    return in + 'a' - 10;
  }
}

bool k_is_uppercase(char ch) {
  return (ch > 64 && ch < 91);
}