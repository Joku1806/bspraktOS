#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#define SP_POSITION 13
#define LR_POSITION 14
#define PC_POSITION 15

typedef struct {
  uint32_t general[13];
  void *sp;
  void *lr;
  void *pc;
} registers;

uint32_t get_current_sp();
void set_current_sp(uint32_t sp);

#endif