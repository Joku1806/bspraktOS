#ifndef HASH_H
#define HASH_H

unsigned hash_string(const char *str);
unsigned hash_string_with_seed(const char *str, unsigned seed);

#endif