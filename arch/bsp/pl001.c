#include <arch/bsp/pl001.h>
#include <config.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kringbuffer.h>

// FIXME: Vieles davon sollte nicht in dieser Datei sein,
// arch sollte jedenfalls nicht auf Funktionalität aus dem Kernel
// zugreifen müssen.
static char buffer[UART_INPUT_BUFFER_SIZE];
static k_ringbuffer cache;

volatile uint32_t *pl001_register(register_offsets offset) {
  return (volatile uint32_t *)(UART_BASE + offset);
}

void pl001_setup() {
  k_ringbuffer_initialise(&cache, buffer, UART_INPUT_BUFFER_SIZE);
  *pl001_register(IMSC) |= RXIM;
}

void pl001_receive() {
  char ch = *pl001_register(DR);
  k_ringbuffer_write(&cache, &ch, 1);
}

bool pl001_has_unread_character() {
  return !k_ringbuffer_empty(&cache);
}

char pl001_read() {
  char ch;
  VERIFY(k_ringbuffer_read(&cache, &ch, 1) == 1);
  return ch;
}

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}