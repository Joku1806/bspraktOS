#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "MMU Setup"

#include <arch/cpu/mmu.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <stddef.h>
#include <stdint.h>

l1_entry get_l1_full_access_entry();
void initialise_l1_table();

__attribute__((aligned(0x4000))) static l1_entry l1_table[L1_TABLE_SIZE];

l1_entry get_l1_full_access_entry(uint32_t physical_base) {
  l1_entry ret = {
      .packed = 0,
  };

  ret.section.base_address = physical_base;
  ret.section.AP1_0 = 0b11;
  ret.section.pad1_set = 1;

  return ret;
}

l1_entry get_l1_fault_entry() {
  l1_entry ret = {
      .fault = {
          .unused = 0,
          .l1_entry_type_fault = 0b00,
      }};

  return ret;
}

void initialise_l1_table() {
  for (size_t i = 0; i < L1_TABLE_SIZE; i++) {
    l1_table[i] = get_l1_full_access_entry(i);
  }

  kdbgln("--- l1_table[1]\npacked = %#010x\nbase = %#010x\npermissions = %#03b\n",
         l1_table[1].packed, l1_table[1].section.base_address, l1_table[1].section.AP2 << 2 | l1_table[1].section.AP1_0);
}

typedef enum {
  no_access = 0b00,
  client = 0b01,
  manager = 0b11,
} domain_mode;

uint32_t DACR_set_domain(uint32_t DACR, uint8_t domain, domain_mode mode) {
  VERIFY(domain <= 15);

  DACR &= ~(0b11 << domain * 2);
  DACR |= mode << domain * 2;

  return DACR;
}

typedef enum {
  ttf_short = 0b0,
  ttf_long = 0b1,
} ttf;

uint32_t TTBCR_set_translation_table_format(uint32_t TTBCR, ttf format) {
  TTBCR &= ~(1 << 31);
  TTBCR |= format << 31;

  return TTBCR;
}

typedef enum {
  MMU_enable = 0,
  cache_enable = 2,
  instruction_cache_enable = 12,
} SCTRL_bit_offset;

uint32_t SCTRL_deactivate_caches(uint32_t SCTRL) {
  return SCTRL & ~(1 << cache_enable | 1 << instruction_cache_enable);
}

uint32_t SCTRL_activate_mmu(uint32_t SCTRL) {
  return SCTRL | 1 << MMU_enable;
}

void mmu_configure() {
  initialise_l1_table();

  uint32_t DACR, TTBCR, SCTRL;
  asm volatile(
      "mrc p15, 0, %0, c3, c0, 0 \n\t"
      "mrc p15, 0, %1, c2, c0, 0 \n\t"
      "mrc p15, 0, %2, c1, c0, 0 \n\t"
      : "=r"(DACR), "=r"(TTBCR), "=r"(SCTRL));

  DACR = DACR_set_domain(DACR, 0, client);
  TTBCR = TTBCR_set_translation_table_format(TTBCR, ttf_short);
  SCTRL = SCTRL_deactivate_caches(SCTRL);
  SCTRL = SCTRL_activate_mmu(SCTRL);

  asm volatile(
      // Adresse von L1 Tabelle an MMU übergeben (TTBR0)
      "mcr p15, 0, %0, c2, c0, 0 \n\t"
      // Domain 0 auf Client Mode setzen (DACR)
      "mcr p15, 0, %1, c3, c0, 0 \n\t"
      // Short Translation Table Format benutzen (TTBCR)
      "mcr p15, 0, %2, c2, c0, 0 \n\t"
      // Instruction und Data Caches deaktivieren, MMU anschalten (SCTLR, Bits C/I/M)
      "mcr p15, 0, %3, c1, c0, 0 \n\t" ::"r"(&l1_table[0]),
      "r"(DACR), "r"(TTBCR), "r"(SCTRL));
}