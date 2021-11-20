#include <arch/bsp/pl001.h>

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

int8_t pl001_receive() {
  while (*pl001_register(FR) & RXFE) {}
  return *pl001_register(DR);
}

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}