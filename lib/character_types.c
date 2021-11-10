#include <kernel/debug.h>
#include <lib/character_types.h>
#include <lib/error_codes.h>

bool is_ascii_decimal_digit(char ch) { return ch >= '0' && ch <= '9'; }
uint8_t parse_ascii_decimal_digit(char in) { return in - '0'; }

char to_ascii_hexadecimal_digit(uint8_t in) {
  if (in <= 9) {
    return in + '0';
  } else if (in <= 15) {
    return in + 'a' - 10;
  }

  warnln("%c is not a hexadecimal digit.\n", in);
  return -EINVAL;
}