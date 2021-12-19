#ifndef DEBUG_H
#define DEBUG_H

#include <arch/cpu/mission_control.h>
#include <kernel/kprintf.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define WARNING_COLOR "\033[33;1m"
#define PANIC_COLOR "\033[31;1m"
#define RESET_COLOR "\033[0m"

#define PANIC_LEVEL 0
#define WARNING_LEVEL 1
#define DEBUG_LEVEL 2

#ifndef LOG_LEVEL
#define LOG_LEVEL DEBUG_LEVEL
#endif

#ifndef LOG_LABEL
#define LOG_LABEL "Unknown"
#endif

#define COLORED_OUTPUT true

// inspiriert von https://stackoverflow.com/questions/8317508/hash-function-for-a-string
#define P1 54059
#define P2 76963
#define SEED_RED 37
#define SEED_GREEN 59
#define SEED_BLUE 83

inline uint8_t label_color_hash(unsigned long seed) {
  unsigned long h = seed;
  for (size_t i = 0; i < sizeof(LOG_LABEL) - 1; i++) {
    h = (h * P1) ^ (LOG_LABEL[i] * P2);
  }

  // wir machen uns damit ein bisschen die Verteilungsqualität
  // kaputt, aber floating point Skalierung können wir leider
  // nicht benutzen :/ Vielleicht fällt mir noch eine bessere
  // Lösung ein.
  return h % UINT8_MAX;
}

#if LOG_LEVEL >= DEBUG_LEVEL
#define dbgln(...)                                                            \
  do {                                                                        \
    if (COLORED_OUTPUT) {                                                     \
      uint8_t cr = label_color_hash(SEED_RED);                                \
      uint8_t cg = label_color_hash(SEED_GREEN);                              \
      uint8_t cb = label_color_hash(SEED_BLUE);                               \
      kprintf("\033[38;2;%u;%u;%um(" LOG_LABEL ") " RESET_COLOR, cr, cg, cb); \
    } else {                                                                  \
      kprintf("(" LOG_LABEL ") ");                                            \
    }                                                                         \
                                                                              \
    kprintf(__VA_ARGS__);                                                     \
    kprintf("\n");                                                            \
  } while (0)
#else
#define dbgln(...)
#endif

#if LOG_LEVEL >= WARNING_LEVEL
#define warnln(...)                                                                                                  \
  do {                                                                                                               \
    if (COLORED_OUTPUT) {                                                                                            \
      uint8_t cr = label_color_hash(SEED_RED);                                                                       \
      uint8_t cg = label_color_hash(SEED_GREEN);                                                                     \
      uint8_t cb = label_color_hash(SEED_BLUE);                                                                      \
      kprintf(WARNING_COLOR "[WARNING] " RESET_COLOR "\033[38;2;%u;%u;%um(" LOG_LABEL ") " RESET_COLOR, cr, cg, cb); \
    } else {                                                                                                         \
      kprintf("[WARNING] (" LOG_LABEL ") ");                                                                         \
    }                                                                                                                \
                                                                                                                     \
    kprintf(__VA_ARGS__);                                                                                            \
    kprintf("\n");                                                                                                   \
  } while (0)
#else
#define warnln(...)
#endif

#if LOG_LEVEL >= PANIC_LEVEL
#define panicln(...)                                                                                             \
  do {                                                                                                           \
    if (COLORED_OUTPUT) {                                                                                        \
      uint8_t cr = label_color_hash(SEED_RED);                                                                   \
      uint8_t cg = label_color_hash(SEED_GREEN);                                                                 \
      uint8_t cb = label_color_hash(SEED_BLUE);                                                                  \
      kprintf(PANIC_COLOR "[PANIC] " RESET_COLOR "\033[38;2;%u;%u;%um(" LOG_LABEL ") " RESET_COLOR, cr, cg, cb); \
    } else {                                                                                                     \
      kprintf("[PANIC] (" LOG_LABEL ") ");                                                                       \
    }                                                                                                            \
                                                                                                                 \
    kprintf(__VA_ARGS__);                                                                                        \
    kprintf("\n");                                                                                               \
    halt_cpu();                                                                                                  \
  } while (0)
#else
#define panic(...)
#endif

#endif