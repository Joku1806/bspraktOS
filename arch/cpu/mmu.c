#define LOG_LEVEL DEBUG_LEVEL
#define LOG_LABEL "MMU Setup"

#include <arch/cpu/mmu.h>
#include <kernel/lib/kassertions.h>
#include <kernel/lib/kdebug.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((aligned(L1_TABLE_SIZE * sizeof(l1_entry)))) static l1_entry l1_table[L1_TABLE_SIZE];

// .init/.text - Kernel: R-X, User: ---, L1
// .data - Kernel: RW?, User: ---, L1
// .rodata - Kernel: R--, User: ---, L1
// .utext - Kernel: RW-, User: R-X, L1
// .udata - Kernel: RW-, User: RW-, L1
// .urodata - Kernel: R--, User: R--, L1
// User Stack - Kernel: RW-, User: RW-, L2
// Interrupt Stack - Kernel: RW-, User: ---, L2
// Guard Page: Fault, L2

#ifdef EXPERIMENTAL
void initialise_l1_table_complete() {
  // QEMU Bereich
  for (size_t i = 0; i < 8; i++) {
    l2_tables[0][i] = get_l2_guard_page();
  }

  // FIXME: Kernel Zeug muss in anderen Bereich verschoben werden,
  // weil wir PXN für den 1. MiB für die User Sektionen setzen müssen
  // .init/.text
  for (size_t i = 8; i < 32; i++) {
    l2_tables[0][i] = get_l2_page(KREAD | KEXEC, UNONE);
  }

  // .rodata
  for (size_t i = 32; i < 80; i++) {
    l2_tables[0][i] = get_l2_page(KREAD, UNONE);
  }

  // .data
  for (size_t i = 80; i < 128; i++) {
    l2_tables[0][i] = get_l2_page(KREAD | KWRITE, UNONE);
  }

  // .utext
  for (size_t i = 128; i < 144; i++) {
    l2_tables[0][i] = get_l2_page(KREAD | KWRITE, UREAD | UEXEC);
  }

  // .urodata
  for (size_t i = 144; i < 192; i++) {
    l2_tables[0][i] = get_l2_page(KREAD, UREAD);
  }

  // .udata
  for (size_t i = 192; i < 256; i++) {
    l2_tables[0][i] = get_l2_page(KREAD | KWRITE, UREAD | UWRITE);
  }
}
#endif

uint32_t DACR_set_domain(uint32_t DACR, uint8_t domain, domain_mode mode) {
  VERIFY(domain <= 15);

  DACR &= ~(0b11 << domain * 2);
  DACR |= mode << domain * 2;

  return DACR;
}

uint32_t TTBCR_set_translation_table_format(uint32_t TTBCR, ttf format) {
  TTBCR &= ~(1 << 31);
  TTBCR |= format << 31;

  return TTBCR;
}

uint32_t SCTRL_deactivate_caches(uint32_t SCTRL) {
  return SCTRL & ~(1 << cache_enable | 1 << instruction_cache_enable);
}

uint32_t SCTRL_activate_mmu(uint32_t SCTRL) {
  return SCTRL | 1 << MMU_enable;
}

uint8_t get_AP_bits(kpermissions kp, upermissions up) {
  switch (COMBINE_PERMISSIONS(kp, up)) {
    case COMBINE_PERMISSIONS(KNONE, UNONE):
      return 0b000;
    case COMBINE_PERMISSIONS(KREAD | KWRITE, UNONE):
      return 0b001;
    case COMBINE_PERMISSIONS(KREAD, UNONE):
      return 0b101;
    case COMBINE_PERMISSIONS(KREAD, UREAD):
      return 0b111;
    case COMBINE_PERMISSIONS(KREAD | KWRITE, UREAD):
      return 0b010;
    case COMBINE_PERMISSIONS(KREAD | KWRITE, UREAD | UWRITE):
      return 0b011;
    default:
      VERIFY_NOT_REACHED();
  }
}

l1_section get_l1_section(uint32_t physical_base, kpermissions kp, upermissions up) {
  VERIFY(physical_base % MiB == 0);

  l1_section ret = {0};

  ret.base_address = physical_base / MiB;
  ret.pad1_set = 1;

  if (!(kp & KEXEC)) {
    ret.PXN = 1;
  }

  if (!(up & UEXEC)) {
    ret.XN = 1;
  }

  uint8_t AP = get_AP_bits(kp, up);
  ret.AP2 = (AP & 0b100) >> 2;
  ret.AP1_0 = (AP & 0b011);

  return ret;
}

l1_fault get_l1_guard_page() {
  l1_fault guard = {0};
  return guard;
}

l2_small_page get_l2_small_page(uint32_t physical_base, kpermissions kp, upermissions up) {
  VERIFY(physical_base % (4 * KiB) == 0);

  l2_small_page ret = {0};

  ret.base_address = physical_base / (4 * KiB);
  ret.pad0_set = 1;

  // Wichtig: PXN wird im l2 Handle gesetzt

  if (~(up & UEXEC)) {
    ret.XN = 1;
  }

  uint8_t AP = get_AP_bits(kp, up);
  ret.AP2 = (AP & 0b100) >> 2;
  ret.AP1_0 = (AP & 0b011);

  return ret;
}

l2_fault get_l2_guard_page() {
  l2_fault guard = {0};
  return guard;
}

void initialise_l1_table() {
  for (size_t i = 0; i < L1_TABLE_SIZE; i++) {
    l1_table[i].section = get_l1_section(i * MiB, KREAD | KWRITE | KEXEC, UREAD | UWRITE | UEXEC);
  }

  kdbgln("--- l1_table[1]\npacked = %#010x\nbase = %#010x\npermissions = %#03b\n",
         l1_table[1].packed, l1_table[1].section.base_address, l1_table[1].section.AP2 << 2 | l1_table[1].section.AP1_0);
}

void mmu_configure() {
  initialise_l1_table();

  uint32_t DACR, TTBCR, SCTLR;
  asm volatile(
      "mrc p15, 0, %0, c3, c0, 0 \n\t"
      "mrc p15, 0, %1, c2, c0, 0 \n\t"
      "mrc p15, 0, %2, c1, c0, 0 \n\t"
      : "=r"(DACR), "=r"(TTBCR), "=r"(SCTLR));

  kdbgln("L1 Tabelle @ %p", l1_table);
  kdbgln("Nach Auslesen: DACR = %#010x, TTBCR = %#010x, SCTLR = %#010x", DACR, TTBCR, SCTLR);

  DACR = DACR_set_domain(DACR, 0, client);
  TTBCR = TTBCR_set_translation_table_format(TTBCR, ttf_short);
  SCTLR = SCTRL_deactivate_caches(SCTLR);
  SCTLR = SCTRL_activate_mmu(SCTLR);

  kdbgln("Nach Konfigurieren: DACR = %#010x, TTBCR = %#010x, SCTLR = %#010x", DACR, TTBCR, SCTLR);

  asm volatile(
      // Adresse von L1 Tabelle an MMU übergeben (TTBR0)
      "mcr p15, 0, %0, c2, c0, 0 \n\t"
      // Domain 0 auf Client Mode setzen (DACR)
      "mcr p15, 0, %1, c3, c0, 0 \n\t"
      // Short Translation Table Format benutzen (TTBCR)
      "mcr p15, 0, %2, c2, c0, 0 \n\t"
      // Instruction und Data Caches deaktivieren, MMU anschalten (SCTLR, Bits C/I/M)
      "mcr p15, 0, %3, c1, c0, 0 \n\t" ::"r"(&l1_table[0]),
      "r"(DACR), "r"(TTBCR), "r"(SCTLR));
}