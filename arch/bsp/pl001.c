#include <arch/bsp/pl001.h>

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

void pl001_wait_until_transmission_complete() {
  while (*pl001_register(FR) & TXFF) {
  }
}

int8_t pl001_receive() {
  if (*pl001_register(FR) & RXFE) {
    return -1;
  }

  return *pl001_register(DR);
}

void pl001_send(char ch) {
  pl001_wait_until_transmission_complete();
  *pl001_register(DR) = ch;
}