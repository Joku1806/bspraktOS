#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "Kernel Start"

#include <arch/bsp/peripherals.h>
#include <arch/bsp/pl001.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/mission_control.h>
#include <config.h>
#include <kernel/lib/kdebug.h>
#include <kernel/lib/kmath.h>
#include <kernel/lib/kprintf.h>
#include <kernel/lib/ktiming.h>
#include <kernel/mmu.h>
#include <kernel/regcheck.h>
#include <kernel/scheduler.h>
#include <stdbool.h>
#include <stddef.h>

#if LOG_COLORED_OUTPUT
void preview_module_colors() {
  const char *modules[] = {"Unknown",        "Interrupt",  "Scheduler", "Kernel Start", "Color",
                           "Intrusive List", "Ringbuffer", "Timing",    "Userthread"};

  kprintf("Previewing Module Colors (WIP)\n");
  for (size_t i = 0; i < sizeof(modules) / sizeof(const char *); i++) {
    long hue_hash = k_clamp_unsigned(k_hash_string_with_seed(modules[i], HUE_SEED), 0, LONG_MAX);
    long saturation_hash =
        k_clamp_unsigned(k_hash_string_with_seed(modules[i], SATURATION_SEED), 0, LONG_MAX);
    long lightness_hash =
        k_clamp_unsigned(k_hash_string_with_seed(modules[i], LIGHTNESS_SEED), 0, LONG_MAX);
    k_hsl_color hsl = {
        .hue = k_project_long_onto_range(hue_hash, 0, 359),
        .saturation = k_project_long_onto_range(saturation_hash, 70, 90),
        .lightness = k_project_long_onto_range(lightness_hash, 50, 80),
    };
    k_rgb_color module_color = k_hsl_to_rgb(&hsl);
    kprintf("hsl(%3u°, %3u%%, %3u%%) -> \033[38;2;%u;%u;%um%s\033[0m\n", hsl.hue, hsl.saturation,
            hsl.lightness, module_color.red, module_color.green, module_color.blue, modules[i]);
  }
  kprintf("\n");
}
#endif

void print_menu() {
  kprintf("Willkommen in unserem Betriebssystem!\n"
          "Um Prozesse und Threads zu testen, gebe Buchstaben ein.\n\n");
}

void start_kernel() {
  pl001_setup();
  mmu_configure();

  scheduler_initialise();

#if LOG_COLORED_OUTPUT
  preview_module_colors();
#endif
  print_menu();

  enable_peripheral_interrupts();
  systimer_reset();
  halt_cpu();
}
