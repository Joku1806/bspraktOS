#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define L1_TABLE_SIZE 0x1000

typedef struct __attribute__((__packed__)) {
  uint32_t l1_entry_type_fault : 2;
  uint32_t unused : 30;
} l1_fault;

typedef struct __attribute__((__packed__)) {
  uint32_t PXN : 1;
  uint32_t pad1_set : 1;
  uint32_t B : 1;
  uint32_t C : 1;
  uint32_t XN : 1;
  uint32_t domain : 4;
  uint32_t IMPL : 1;
  uint32_t AP1_0 : 2;
  uint32_t TEX : 3;
  uint32_t AP2 : 1;
  uint32_t S : 1;
  uint32_t nG : 1;
  uint32_t pad0_unset : 1;
  uint32_t NS : 1;
  uint32_t base_address : 12;
} l1_section;

typedef struct __attribute__((__packed__)) {
  uint32_t pad1_set : 1;
  uint32_t pad0_unset : 1;
  uint32_t PXN : 1;
  uint32_t NS : 1;
  uint32_t SBZ : 1;
  uint32_t domain : 4;
  uint32_t IMPL : 1;
  uint32_t base_address : 22;
} l2_handle;

typedef struct __attribute__((__packed__)) {
  uint32_t l2_entry_type_fault : 2;
  uint32_t unused : 30;
} l2_fault;

typedef struct __attribute__((__packed__)) {
  uint32_t l2_entry_type_lp : 2;
  uint32_t B : 1;
  uint32_t C : 1;
  uint32_t AP1_0 : 2;
  uint32_t SBZ : 3;
  uint32_t AP2 : 1;
  uint32_t S : 1;
  uint32_t nG : 1;
  uint32_t TEX : 3;
  uint32_t XN : 1;
  uint32_t base_address : 16;
} l2_large_page;

typedef struct __attribute__((__packed__)) {
  uint32_t XN : 1;
  uint32_t pad0_set : 1;
  uint32_t B : 1;
  uint32_t C : 1;
  uint32_t AP1_0 : 2;
  uint32_t TEX : 3;
  uint32_t AP2 : 1;
  uint32_t S : 1;
  uint32_t nG : 1;
  uint32_t base_address : 20;
} l2_small_page;

typedef union {
  l2_fault fault;
  l2_large_page large_page;
  l2_small_page small_page;
  uint32_t packed;
} l2_entry;

typedef union {
  l1_fault fault;
  l1_section section;
  l2_handle handle;
  uint32_t packed;
} l1_entry;

void mmu_configure();

#endif