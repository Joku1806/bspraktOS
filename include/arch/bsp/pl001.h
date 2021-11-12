#ifndef PL001_H
#define PL001_H

#include <stdint.h>

#define UART_BASE (0x7E201000 - 0x3F000000)

// wir brauchen vermutlich nicht alle -- nutzlose noch entfernen
typedef enum {
  DR = 0x0,
  FR = 0x18,
  IBRD = 0x24,
  FBRD = 0x28,
  LCRH = 0x2c,
  CR = 0x30,
  IFLS = 0x34,
  IMSC = 0x38,
  RIS = 0x3c,
  MIS = 0x40,
  ICR = 0x44,
  DMACR = 0x48,
  ITCR = 0x80,
  ITIP = 0x84,
  ITOP = 0x88,
  TDR = 0x8c
} register_offsets;

typedef enum {
  BUSY = 1 << 3,
  RXFE = 1 << 4,
  TXFF = 1 << 5,
} FR_FLAGS;

int8_t pl001_receive();
void pl001_send(char ch);

#endif