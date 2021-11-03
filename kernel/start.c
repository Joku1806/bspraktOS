#include "kernel/kprintf.h"
#include <arch/bsp/yellow_led.h>
#include <config.h>
#include <kernel/drivers/pl001.h>

volatile unsigned int counter = 0;

void increment_counter() { counter++; }

void start_kernel() {

  yellow_on();
  test_kprintf();

  // Endless counter
  for (;;) {
    increment_counter();
    int8_t ch = pl001_receive();
    if (ch < 0) {
      continue;
    }

    kprintf("Es wurde folgender Buchstabe eingegeben: %c, In Hexadezimal: %x, "
            "In Dezimal: %08u\n",
            ch, ch, ch);
  }
}
