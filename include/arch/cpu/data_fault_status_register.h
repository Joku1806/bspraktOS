#ifndef DATA_FAULT_STATUS_REGISTER_H
#define DATA_FAULT_STATUS_REGISTER_H

#include <stdint.h>

typedef enum {
  dfsr_status = 0xf << 0,
  dfsr_domain = 0xf << 4,
  dfsr_FS = 0x1 << 10,
  dfsr_WnR = 0x1 << 11,
  dfsr_ExT = 0x1 << 12
} dfsr_flags;

typedef enum {
  dfsr_alignment_fault = 0x1,
  dfsr_instruction_cache_maintenance_fault = 0x4,
  dfsr_synchronous_external_abort_1st_level_translation = 0xc,
  dfsr_synchronous_external_abort_2nd_level_translation = 0xe,
  dfsr_section_translation_fault = 0x5,
  dfsr_page_translation_fault = 0x7,
  dfsr_section_access_flag_fault = 0x3,
  dfsr_page_access_flag_fault = 0x6,
  dfsr_section_domain_fault = 0x9,
  dfsr_page_domain_fault = 0xb,
  dfsr_section_permission_fault = 0xd,
  dfsr_page_permission_fault = 0xf,
  dfsr_nontranslation_synchronous_external_abort = 0x8,
  dfsr_asynchronous_external_abort = 0x6,
  dfsr_debug_event = 0x2,
} dfsr_status_bits;

typedef enum { WnR_read = 0x0, WnR_write = 0x1 } dfsr_WnR_bits;

typedef enum { ExT_DECERR = 0x0, ExT_SLVERR = 0x1 } dfsr_ExT_bits;

const char *get_data_abort_error_type(uint32_t dfsr);
const char *get_data_abort_access_type(uint32_t dfsr);

#endif