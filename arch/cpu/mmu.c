#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "MMU Setup"

#include <arch/bsp/memory_map.h>
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

bool l1_entry_is_section(l1_entry *e) {
  return e->section.pad1_set && !e->section.pad0_unset;
}

void l1_section_set_base_address(l1_section *s, uint32_t base_address) {
  VERIFY(base_address % MiB == 0);
  s->base_address = base_address / MiB;
}

void initialise_l1_table() {
  uint32_t address_range_starts[DISTINCT_SECTIONS_COUNT + 1] = {
      UNASSIGNED0_START_ADDRESS,
      INIT_SECTION_START_ADDRESS,
      TEXT_SECTION_START_ADDRESS,
      BSS_SECTION_START_ADDRESS,
      RODATA_SECTION_START_ADDRESS,
      DATA_SECTION_START_ADDRESS,
      UTEXT_SECTION_START_ADDRESS,
      URODATA_SECTION_START_ADDRESS,
      UDATA_SECTION_START_ADDRESS,
      UNASSIGNED1_START_ADDRESS,
      MMIO_DEVICES_START_ADDRESS,
      UNASSIGNED2_START_ADDRESS,
      USER_STACK_BOTTOM_ADDRESS,
      KERNEL_STACK_BOTTOM_ADDRESS,
      MEMORY_TOP_ADDRESS,
  };

  l1_entry sections[DISTINCT_SECTIONS_COUNT] = {
      {.fault = get_l1_guard_page()},
      {.section = get_l1_section(INIT_SECTION_START_ADDRESS, KREAD | KEXEC, UNONE)},
      {.section = get_l1_section(TEXT_SECTION_START_ADDRESS, KREAD | KEXEC, UNONE)},
      {.section = get_l1_section(BSS_SECTION_START_ADDRESS, KREAD | KWRITE, UNONE)},
      {.section = get_l1_section(RODATA_SECTION_START_ADDRESS, KREAD, UNONE)},
      {.section = get_l1_section(DATA_SECTION_START_ADDRESS, KREAD | KWRITE, UNONE)},
      {.section = get_l1_section(UTEXT_SECTION_START_ADDRESS, KREAD | KWRITE, UREAD | UEXEC)},
      {.section = get_l1_section(URODATA_SECTION_START_ADDRESS, KREAD, UREAD)},
      {.section = get_l1_section(UDATA_SECTION_START_ADDRESS, KREAD | KWRITE, UREAD | UWRITE)},
      {.fault = get_l1_guard_page()},
      {.section = get_l1_section(MMIO_DEVICES_START_ADDRESS, KREAD | KWRITE, UNONE)},
      {.fault = get_l1_guard_page()},
      {.handle = get_l2_handle(USER_STACK_BOTTOM_ADDRESS, KREAD | KWRITE, UREAD | UWRITE)},
      {.handle = get_l2_handle(KERNEL_STACK_BOTTOM_ADDRESS, KREAD | KWRITE, UNONE)},
  };

  for (size_t i = 0; i < DISTINCT_SECTIONS_COUNT; i++) {
    l1_entry template = sections[i];

    for (size_t j = address_range_starts[i]; j < address_range_starts[i + 1]; j += MiB) {
      if (l1_entry_is_section(&template)) {
        l1_section_set_base_address(&(template.section), j);
      }

      l1_table[j / MiB] = template;
    }
  }
}

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
  l1_section ret = {0};
  ret.pad1_set = 1;

  l1_section_set_base_address(&ret, physical_base);

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

void mmu_configure() {
  initialise_l1_table();

  uint32_t DACR, TTBCR, SCTLR, TTBR0;
  asm volatile(
      "mrc p15, 0, %0, c3, c0, 0 \n\t"
      "mrc p15, 0, %1, c2, c0, 2 \n\t"
      "mrc p15, 0, %2, c1, c0, 0 \n\t"
      : "=r"(DACR), "=r"(TTBCR), "=r"(SCTLR));

  kdbgln("L1 Tabelle @ %p", l1_table);
  kdbgln("Nach Auslesen: DACR = %#010x, TTBCR = %#010x, SCTLR = %#010x", DACR, TTBCR, SCTLR);

  DACR = DACR_set_domain(DACR, 0, client);
  TTBCR = TTBCR_set_translation_table_format(TTBCR, ttf_short);
  SCTLR = SCTRL_deactivate_caches(SCTLR);
  SCTLR = SCTRL_activate_mmu(SCTLR);

  asm volatile(
      // Adresse von L1 Tabelle an MMU übergeben (TTBR0)
      "mcr p15, 0, %0, c2, c0, 0 \n\t"
      // Domain 0 auf Client Mode setzen (DACR)
      "mcr p15, 0, %1, c3, c0, 0 \n\t"
      // Short Translation Table Format benutzen (TTBCR)
      "mcr p15, 0, %2, c2, c0, 2 \n\t"
      // Instruction und Data Caches deaktivieren, MMU anschalten (SCTLR, Bits C/I/M)
      "mcr p15, 0, %3, c1, c0, 0 \n\t" ::"r"(&l1_table[0]),
      "r"(DACR), "r"(TTBCR), "r"(SCTLR));

  asm volatile(
      "mrc p15, 0, %0, c2, c0, 0 \n\t"
      "mrc p15, 0, %1, c3, c0, 0 \n\t"
      "mrc p15, 0, %2, c2, c0, 2 \n\t"
      "mrc p15, 0, %3, c1, c0, 0 \n\t"
      ""
      : "=r"(TTBR0), "=r"(DACR), "=r"(TTBCR), "=r"(SCTLR));

  kdbgln("Nach Konfigurieren: DACR = %#010x, TTBCR = %#010x, SCTLR = %#010x", DACR, TTBCR, SCTLR);
}