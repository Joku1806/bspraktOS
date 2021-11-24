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

void pl001_receive() {
  ringbuffer_write(&pl001_buffer_internal, *pl001_register(DR));
}

char pl001_read(){
  while(!pl001_buffer_internal.valid_reads){}
  return ringbuffer_read(&pl001_buffer_internal);
}

void pl001_send(char ch) {
  while (*pl001_register(FR) & TXFF) {}
  *pl001_register(DR) = ch;
}