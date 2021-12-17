#include <arch/bsp/pl001.h>
#include <config.h>
#include <lib/ringbuffer.h>

char pl001_buffer_contents_internal[UART_INPUT_BUFFER_SIZE];
ringbuffer pl001_buffer_internal;

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

void pl001_setup() {
  pl001_buffer_internal =
      ringbuffer_create(pl001_buffer_contents_internal, UART_INPUT_BUFFER_SIZE);
  *pl001_register(IMSC) |= RXIM;
}

void pl001_receive() {
  ringbuffer_write(&pl001_buffer_internal, *pl001_register(DR));
}

bool pl001_has_unread_character() {
  return ringbuffer_write_occured(&pl001_buffer_internal);
}

char pl001_read() { return ringbuffer_read(&pl001_buffer_internal); }

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}