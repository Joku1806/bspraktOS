#ifndef DEBUG_H
#define DEBUG_H

#include <arch/cpu/mission_control.h>
#include <kernel/kprintf.h>

#define DEBUG_COLOR "\033[94m"
#define WARN_COLOR "\033[33;1m"
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

#if LOG_LEVEL >= DEBUG_LEVEL
#define dbgln(...)                                                             \
  do {                                                                         \
    kprintf(DEBUG_COLOR "[*] " RESET_COLOR "(" LOG_LABEL "): ");               \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\n");                                                             \
  } while (0)
#else
#define dbgln(...)
#endif

#if LOG_LEVEL >= WARNING_LEVEL
#define warnln(...)                                                            \
  do {                                                                         \
    kprintf(WARN_COLOR "[WARNING] " RESET_COLOR "(" LOG_LABEL "): ");          \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\n");                                                             \
  } while (0)
#else
#define warnln(...)
#endif

#if LOG_LEVEL >= PANIC_LEVEL
#define panicln(...)                                                           \
  do {                                                                         \
    kprintf(PANIC_COLOR "[PANIC] " RESET_COLOR "(" LOG_LABEL "): ");           \
    kprintf(__VA_ARGS__);                                                      \
    kprintf("\n");                                                             \
    halt_cpu();                                                                \
  } while (0)
#else
#define panic(...)
#endif

#endif