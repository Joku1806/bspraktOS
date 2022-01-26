#include <arch/cpu/mmu.h>
#include <stddef.h>
#include <stdint.h>

__attribute__((aligned(0x4000))) static l1_entry l1_table[L1_TABLE_SIZE];

l1_entry get_full_access_entry(uint32_t physical_base) {
  l1_entry ret = {
      .section = {
          .base_address = physical_base,
          .NS = 0,
          .pad0_unset = 0,
          .nG = 0,
          .S = 0,
          // FIXME: besser benennen
          .AP0 = 0,
          .TEX = 0,
          .AP1 = 0b11,
          .IMPL = 0,
          .domain = 0,
          .XN = 0,
          .C = 0,
          .B = 0,
          .pad1_set = 1,
          .PXN = 0,
      }};

  return ret;
}

void initialise_l1_table() {
  for (size_t i = 0; i < L1_TABLE_SIZE; i++) {
    l1_table[i] = get_full_access_entry(i);
  }
}