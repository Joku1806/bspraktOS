#include <kernel/lib/khash.h>
#include <kernel/lib/kmath.h>
#include <kernel/lib/kstring.h>
#include <stddef.h>

// Jenkins Hash Implementierung kopiert von
// https://github.com/troydhanson/uthash/blob/master/src/uthash.h
unsigned k_hash_jen(void *key, unsigned keylen, unsigned seed);

// clang-format off
#define K_HASH_JEN_MIX(a, b, c)       \
do {                                \
  a -= b; a -= c; a ^= ( c >> 13 ); \
  b -= c; b -= a; b ^= ( a << 8 );  \
  c -= a; c -= b; c ^= ( b >> 13 ); \
  a -= b; a -= c; a ^= ( c >> 12 ); \
  b -= c; b -= a; b ^= ( a << 16 ); \
  c -= a; c -= b; c ^= ( b >> 5 );  \
  a -= b; a -= c; a ^= ( c >> 3 );  \
  b -= c; b -= a; b ^= ( a << 10 ); \
  c -= a; c -= b; c ^= ( b >> 15 ); \
} while (0)
// clang-format on

unsigned k_hash_jen(void *key, unsigned keylen, unsigned seed) {
  unsigned _hj_i, _hj_j, _hj_k;
  unsigned const char *_hj_key = (unsigned const char *)(key);
  unsigned hashv = seed;
  _hj_i = _hj_j = 0x9e3779b9u;
  _hj_k = (unsigned)(keylen);

  while (_hj_k >= 12U) {
    _hj_i += (_hj_key[0] + ((unsigned)_hj_key[1] << 8) + ((unsigned)_hj_key[2] << 16) + ((unsigned)_hj_key[3] << 24));
    _hj_j += (_hj_key[4] + ((unsigned)_hj_key[5] << 8) + ((unsigned)_hj_key[6] << 16) + ((unsigned)_hj_key[7] << 24));
    hashv += (_hj_key[8] + ((unsigned)_hj_key[9] << 8) + ((unsigned)_hj_key[10] << 16) + ((unsigned)_hj_key[11] << 24));

    K_HASH_JEN_MIX(_hj_i, _hj_j, hashv);

    _hj_key += 12;
    _hj_k -= 12U;
  }

  hashv += (unsigned)(keylen);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch (_hj_k) {
    case 11:
      hashv += ((unsigned)_hj_key[10] << 24);
    case 10:
      hashv += ((unsigned)_hj_key[9] << 16);
    case 9:
      hashv += ((unsigned)_hj_key[8] << 8);
    case 8:
      _hj_j += ((unsigned)_hj_key[7] << 24);
    case 7:
      _hj_j += ((unsigned)_hj_key[6] << 16);
    case 6:
      _hj_j += ((unsigned)_hj_key[5] << 8);
    case 5:
      _hj_j += _hj_key[4];
    case 4:
      _hj_i += ((unsigned)_hj_key[3] << 24);
    case 3:
      _hj_i += ((unsigned)_hj_key[2] << 16);
    case 2:
      _hj_i += ((unsigned)_hj_key[1] << 8);
    case 1:
      _hj_i += _hj_key[0];
    default:;
  }
#pragma GCC diagnostic pop

  K_HASH_JEN_MIX(_hj_i, _hj_j, hashv);
  return hashv;
}

unsigned k_hash_string(const char *str) {
  return k_hash_jen((void *)str, k_strlen(str), 0xfeedbeef);
}

unsigned k_hash_string_with_seed(const char *str, unsigned seed) {
  return k_hash_jen((void *)str, k_strlen(str), seed);
}