#ifndef DEBUG_H
#define DEBUG_H

#include "kprintf.h"

#define DEBUG_COLOR "\033[94m"
#define WARN_COLOR "\033[33;1m"
#define RESET_COLOR "\033[0m"

#ifdef DEBUG
#define dbgln(...)                                                             \
  do {                                                                         \
    kprintf(DEBUG_COLOR "[*] " RESET_COLOR);                                   \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\n");                                                             \
  } while (0)
#else
#define dbgln(...)                                                             \
  do {                                                                         \
  } while (0)
#endif

#define warnln(...)                                                            \
  do {                                                                         \
    kprintf(WARN_COLOR "[WARNING] " RESET_COLOR);                              \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\n");                                                             \
  } while (0)
#endif