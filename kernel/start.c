#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Kernel Start"

#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <config.h>
#include <kernel/kprintf.h>
#include <kernel/regcheck.h>
#include <kernel/thread.h>
#include <lib/debug.h>
#include <lib/timing.h>
#include <stdbool.h>
#include <stddef.h>

void preview_module_colors() {
  const char *modules[] = {
      "Unknown",
      "Interrupt",
      "Scheduler",
      "Kernel Start",
      "Thread",
      "Color",
      "Intrusive List",
      "Ringbuffer",
      "Timing",
      "Userthread"};

  kprintf("Previewing Module Colors\n");
  for (size_t i = 0; i < sizeof(modules) / sizeof(const char *); i++) {
    long hue_hash = clamp_unsigned(hash_string_with_seed(modules[i], HUE_SEED), 0, LONG_MAX);
    long saturation_hash = clamp_unsigned(hash_string_with_seed(modules[i], SATURATION_SEED), 0, LONG_MAX);
    long lightness_hash = clamp_unsigned(hash_string_with_seed(modules[i], LIGHTNESS_SEED), 0, LONG_MAX);
    hsl_color hsl = {
        .hue = project_long_onto_range(hue_hash, 0, 359),
        .saturation = project_long_onto_range(saturation_hash, 70, 90),
        .lightness = project_long_onto_range(lightness_hash, 50, 80),
    };
    rgb_color module_color = hsl_to_rgb(&hsl);
    kprintf("\033[38;2;%u;%u;%um%s\033[0m\n", module_color.red, module_color.green, module_color.blue, modules[i]);
  }
}

void print_menu() {
  kprintf("Willkommen in unserem Betriebssystem!\n"
          "Interrupts ==========================\n"
          "a: (Thread) Data Abort auslösen\n"
          "p: (Thread) Prefetch Abort auslösen\n"
          "s: (Thread) Software Interrupt auslösen\n"
          "u: (Thread) Undefined Instruction auslösen\n"
          "A: (System) Data Abort auslösen\n"
          "P: (System) Prefetch Abort auslösen\n"
          "S: (System) Software Interrupt auslösen\n"
          "U: (System) Undefined Instruction auslösen\n"
          "Debughilfen =========================\n"
          "c: (Thread) Registerchecker ausführen\n\n");
}

void start_kernel() {
  // FIXME: Kernel Thread?
  systimer_reset();
  pl001_setup();
  thread_list_initialise();
  print_menu();
  preview_module_colors();
}