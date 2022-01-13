#ifndef CHARACTER_TYPES_H
#define CHARACTER_TYPES

#include <stdbool.h>
#include <stdint.h>

// Prüft, ob ch eine dezimale Ziffer ist
bool is_ascii_decimal_digit(char ch);

// Konviert die ASCII-Repräsentation einer dezimalen Ziffer zu der eigentlichen
// Zahl.
uint8_t parse_ascii_decimal_digit(char ch);

// Konviert eine hexadezimale Ziffer zu der korrespondierenden
// ASCII-Repräsentation.
char to_ascii_hexadecimal_digit(uint8_t in);

bool is_uppercase(char ch);

#endif