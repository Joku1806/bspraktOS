#ifndef INTERRUPT_PERIPHERALS_H
#define INTERRUPT_PERIPHERALS_H

#define INTERRUPT_PERIPHERALS_BASE (0x7E00B000 - 0x3F000000)

#ifdef __ASSEMBLER__

#define ENABLE_IRQS_1_OFFSET 0x210
#define TIMER1_ENABLE_FLAG 0x2
#define ENABLE_IRQS_2_OFFSET 0x214
#define INTERRUPT_ENABLE_FLAG 1<<25

#else

#include <stdint.h>

typedef enum {
  IRQ_basic_pending = 0x200,
  IRQ_pending_1 = 0x204,
  IRQ_pending_2 = 0x208,
  FIQ_control = 0x20C,
  enable_IRQs_1 = 0x210,
  enable_IRQs_2 = 0x214,
  enable_basic_IRQs = 0x218,
  disable_IRQs_1 = 0x21C,
  disable_IRQs_2 = 0x220,
  disable_basic_IRQs = 0x224
} peripherals_register_offsets;

typedef enum {
  timer1_pending = 0x1 << 1,
} IRQ1_pending_flags;

typedef enum {
  UART_pending = 0x1 << 25,
} IRQ2_pending_flags;


volatile uint32_t *peripherals_register(peripherals_register_offsets offset);

#endif

#endif