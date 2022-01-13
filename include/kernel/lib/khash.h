#ifndef KHASH_H
#define KHASH_H

unsigned k_hash_string(const char *str);
unsigned k_hash_string_with_seed(const char *str, unsigned seed);

#endif