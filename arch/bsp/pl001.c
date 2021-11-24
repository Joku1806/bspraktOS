#include <arch/bsp/pl001.h>
#include <config.h>
#include <lib/ringbuffer.h>

// TODO: Stacks in kernel.lds platzieren, damit das niemals ausversehen
// überschrieben werden kann!
char pl001_buffer_contents_internal[UART_INPUT_BUFFER_SIZE];
ringbuffer pl001_buffer_internal;

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

void pl001_setup() {
  pl001_buffer_internal =
      ringbuffer_create(pl001_buffer_contents_internal, UART_INPUT_BUFFER_SIZE);
}

int8_t pl001_receive() {
  while (*pl001_register(FR) & RXFE) {}
  return *pl001_register(DR);
}

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}