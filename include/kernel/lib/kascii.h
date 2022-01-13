#ifndef KASCII_H
#define KASCII_H

#include <stdbool.h>
#include <stdint.h>

// Prüft, ob ch eine dezimale Ziffer ist
bool k_is_ascii_decimal_digit(char ch);

// Konviert die ASCII-Repräsentation einer dezimalen Ziffer zu der eigentlichen
// Zahl.
uint8_t k_parse_ascii_decimal_digit(char ch);

// Konviert eine hexadezimale Ziffer zu der korrespondierenden
// ASCII-Repräsentation.
char k_to_ascii_hexadecimal_digit(uint8_t in);

bool k_is_uppercase(char ch);

#endif