#ifndef INSTRUCTION_FAULT_STATUS_REGISTER_H
#define INSTRUCTION_FAULT_STATUS_REGISTER_H

#include <stdint.h>

typedef enum {
  ifsr_FS0_3 = 0xf,
  ifsr_LPAE = 0x1 << 9,
  ifsr_FS4 = 0x1 << 10,
  ifsr_ExT = 0x1 << 12,
  ifsr_FnV = 0x1 << 16
} isfr_flags;

typedef enum {
  PC_alignment_fault = 0x1,
  debug_exception = 0x2,
  access_flag_fault_level_1 = 0x3,
  translation_fault_level_1 = 0x5,
  access_flag_fault_level_2 = 0x6,
  translation_fault_level_2 = 0x7,
  synchronous_external_abort_not_on_translation_table_walk = 0x8,
  domain_fault_level_1 = 0x9,
  domain_fault_level_2 = 0xb,
  synchronous_external_abort_on_translation_table_walk_level_1 = 0xc,
  permission_fault_level_1 = 0xd,
  synchronous_external_abort_on_translation_table_walk_level_2 = 0xe,
  permission_fault_level_2 = 0xf,
  TLB_conflict_abort = 0x10,
  IMPLEMENTATION_DEFINED_fault_lockdown_fault = 0x14,
  synchronous_parity_or_ECC_error_on_memory_access_not_on_translation_table_walk =
      0x19,
  synchronous_parity_or_ECC_error_on_translation_table_walk_level_1 = 0x1c,
  synchronous_parity_or_ECC_error_on_translation_table_walk_level_2 = 0x1e
} isfr_fault_status;

const char *get_prefetch_abort_error_type(uint32_t ifsr);

#endif