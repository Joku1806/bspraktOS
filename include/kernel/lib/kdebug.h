#ifndef KDEBUG_H
#define KDEBUG_H

#include <arch/cpu/mission_control.h>
#include <kernel/lib/kprintf.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define PANIC_LEVEL 0
#define WARNING_LEVEL 1
#define DEBUG_LEVEL 2

#ifndef LOG_LEVEL
#define LOG_LEVEL DEBUG_LEVEL
#endif

#ifndef LOG_LABEL
#define LOG_LABEL "Unknown"
#endif

#ifndef LOG_COLORED_OUTPUT
#define LOG_COLORED_OUTPUT true
#endif

#if LOG_COLORED_OUTPUT

#include <kernel/lib/kcolor.h>
#include <kernel/lib/kfraction.h>
#include <kernel/lib/khash.h>
#include <kernel/lib/kmath.h>
#include <limits.h>

#define ESC_RESET_COLOR "\033[0m"

// FIXME: vllt bessere Farben finden
static const k_rgb_color debug_warning_color = {.red = 249, .green = 115, .blue = 31};
static const k_rgb_color debug_panic_color = {.red = 236, .green = 55, .blue = 19};

static k_rgb_color module_color;
static bool initialised_module_color = false;

#define HUE_SEED 0xf9136546
#define SATURATION_SEED 0x70b5bb43
#define LIGHTNESS_SEED 0x8a4ef7c5

#define INITIALISE_MODULE_COLOR_ONCE()                                                                           \
  do {                                                                                                           \
    if (!initialised_module_color) {                                                                             \
      long hue_hash = k_clamp_unsigned(k_hash_string_with_seed(LOG_LABEL, HUE_SEED), 0, LONG_MAX);               \
      long saturation_hash = k_clamp_unsigned(k_hash_string_with_seed(LOG_LABEL, SATURATION_SEED), 0, LONG_MAX); \
      long lightness_hash = k_clamp_unsigned(k_hash_string_with_seed(LOG_LABEL, LIGHTNESS_SEED), 0, LONG_MAX);   \
      k_hsl_color hsl = {                                                                                        \
          .hue = k_project_long_onto_range(hue_hash, 0, 359),                                                    \
          .saturation = k_project_long_onto_range(saturation_hash, 70, 90),                                      \
          .lightness = k_project_long_onto_range(lightness_hash, 50, 80),                                        \
      };                                                                                                         \
      module_color = k_hsl_to_rgb(&hsl);                                                                         \
      initialised_module_color = true;                                                                           \
    }                                                                                                            \
  } while (0)

#define PRINT_COLORED(str, color)                                                           \
  do {                                                                                      \
    kprintf("\033[38;2;%u;%u;%um" str ESC_RESET_COLOR, color.red, color.green, color.blue); \
  } while (0)

#define PRINT_COLORED_LABEL()                        \
  do {                                               \
    INITIALISE_MODULE_COLOR_ONCE();                  \
    PRINT_COLORED("(" LOG_LABEL ") ", module_color); \
  } while (0)

#endif

#if LOG_LEVEL >= DEBUG_LEVEL

#if LOG_COLORED_OUTPUT
#define kdbgln(...)        \
  do {                     \
    PRINT_COLORED_LABEL(); \
    kprintf(__VA_ARGS__);  \
    kprintf("\n");         \
  } while (0)
#else
#define kdbgln(...)              \
  do {                           \
    kprintf("(" LOG_LABEL ") "); \
    kprintf(__VA_ARGS__);        \
    kprintf("\n");               \
  } while (0)
#endif

#else
#define kdbgln(...)
#endif

#if LOG_LEVEL >= WARNING_LEVEL

#if LOG_COLORED_OUTPUT
#define kwarnln(...)                                  \
  do {                                                \
    PRINT_COLORED("[WARNING] ", debug_warning_color); \
    PRINT_COLORED_LABEL();                            \
    kprintf(__VA_ARGS__);                             \
    kprintf("\n");                                    \
  } while (0)
#else
#define kwarnln(...)                       \
  do {                                     \
    kprintf("[WARNING] (" LOG_LABEL ") "); \
    kprintf(__VA_ARGS__);                  \
    kprintf("\n");                         \
  } while (0)
#endif

#else
#define kwarnln(...)
#endif

#if LOG_LEVEL >= PANIC_LEVEL

#if LOG_COLORED_OUTPUT
#define kpanicln(...)                             \
  do {                                            \
    PRINT_COLORED("[PANIC] ", debug_panic_color); \
    PRINT_COLORED_LABEL();                        \
    kprintf(__VA_ARGS__);                         \
    kprintf("\nHalting System.\n");               \
    halt_cpu();                                   \
  } while (0)
#else
#define kpanicln(...)                    \
  do {                                   \
    kprintf("[PANIC] (" LOG_LABEL ") "); \
    kprintf(__VA_ARGS__);                \
    kprintf("\nHalting System.\n");      \
    halt_cpu();                          \
  } while (0)

#endif

#else
#define kpanicln(...)
#endif

#endif