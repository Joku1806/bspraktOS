#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "ASCII Conversion"

#include <user/lib/assertions.h>
#include <user/lib/character_types.h>

bool is_ascii_decimal_digit(char ch) { return ch >= '0' && ch <= '9'; }

uint8_t parse_ascii_decimal_digit(char ch) {
  VERIFY(ch >= '0' && ch <= '9');
  return ch - '0';
}

char to_ascii_hexadecimal_digit(uint8_t in) {
  VERIFY(in <= 15);

  if (in <= 9) {
    return in + '0';
  } else {
    return in + 'a' - 10;
  }
}

bool is_uppercase(char ch) {
  return (ch < 91 && ch > 64);
}