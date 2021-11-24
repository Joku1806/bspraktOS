#include <arch/bsp/interrupt_peripherals.h>

volatile uint32_t *peripherals_register(peripherals_register_offsets offset) {
  return (volatile uint32_t *)(INTERRUPT_PERIPHERALS_BASE + offset);
}