#ifndef CHARACTER_TYPES_H
#define CHARACTER_TYPES

#include <stdbool.h>
#include <stdint.h>

// Prüft, ob ch eine dezimale Ziffer ist
bool is_ascii_decimal_digit(char ch);

// Konviert die ASCII-Repräsentation einer Ziffer zu der eigentlichen Ziffer. Es
// wird angenommen, dass bei unbekanntem Input vorher is_ascii_decimal_digit()
// aufgerufen wurde.
uint8_t parse_ascii_decimal_digit(char in);

// Konviert eine hexadezimale Ziffer zu der korrespondierenden
// ASCII-Repräsentation. Gibt -EINVAL zurück, falls in keine hexadezimale Ziffer
// ist.
char to_ascii_hexadecimal_digit(uint8_t in);

#endif