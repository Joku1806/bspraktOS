#include <arch/bsp/pl001.h>
#include <config.h>
#include <lib/assertions.h>
#include <lib/ringbuffer.h>

static char buffer[UART_INPUT_BUFFER_SIZE];
static ringbuffer cache;

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

void pl001_setup() {
  ringbuffer_initialise(&cache, buffer, UART_INPUT_BUFFER_SIZE);
  *pl001_register(IMSC) |= RXIM;
}

void pl001_receive() {
  char ch = *pl001_register(DR);
  ringbuffer_write(&cache, &ch, 1);
}

bool pl001_has_unread_character() {
  return !ringbuffer_empty(&cache);
}

char pl001_read() {
  char ch;
  VERIFY(ringbuffer_read(&cache, &ch, 1) == 1);
  return ch;
}

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}