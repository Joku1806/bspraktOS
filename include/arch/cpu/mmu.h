#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#define L1_TABLE_SIZE 0x1000

typedef struct __attribute__((__packed__)) {
  uint32_t pad1_unset : 1;
  uint32_t pad0_unset : 1;
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

typedef enum {
  no_access = 0b00,
  client = 0b01,
  manager = 0b11,
} domain_mode;

typedef enum {
  ttf_short = 0b0,
  ttf_long = 0b1,
} ttf;

typedef enum {
  MMU_enable = 0,
  cache_enable = 2,
  instruction_cache_enable = 12,
} SCTRL_bit_offset;

typedef enum {
  KREAD = 1 << 2,
  KWRITE = 1 << 1,
  KEXEC = 1 << 0,
  KNONE = 0,
} kpermissions;

typedef enum {
  UREAD = 1 << 2,
  UWRITE = 1 << 1,
  UEXEC = 1 << 0,
  UNONE = 0,
} upermissions;

#define COMBINE_PERMISSIONS(kp, up) \
  ((kp) & ~KEXEC) << 3 | ((up) & ~UEXEC)

uint32_t DACR_set_domain(uint32_t DACR, uint8_t domain, domain_mode mode);
uint32_t TTBCR_set_translation_table_format(uint32_t TTBCR, ttf format);
uint32_t SCTRL_deactivate_caches(uint32_t SCTRL);
uint32_t SCTRL_activate_mmu(uint32_t SCTRL);

uint8_t get_AP_bits(kpermissions kp, upermissions up);
l1_section get_l1_section(uint32_t physical_base, kpermissions kp, upermissions up);
l1_fault get_l1_guard_page();
l2_small_page get_l2_small_page(uint32_t physical_base, kpermissions kp, upermissions up);
l2_fault get_l2_guard_page();

void mmu_configure();

#endif