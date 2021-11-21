#include<stdint.h>

#define UART_TIMER (0x7E003000 - 0x3F000000)


typedef enum {
  CS = 0x0,
  C1 = 0x14
} systimer_offsets;

typedef enum {
  M1 = 1 << 1
} control_flags;


void start_systimer();

volatile uint32_t *load_register(systimer_offsets offset);
