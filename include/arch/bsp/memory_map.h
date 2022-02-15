#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <arch/bsp/memory_sizes.h>

#define MEMORY_TOP_ADDRESS (128 * MiB)
#define DISTINCT_SECTIONS_COUNT (14)

#define UNASSIGNED0_START_ADDRESS (0)

#define INIT_TEXT_SECTION_START_ADDRESS (1 * MiB)
#define RODATA_SECTION_START_ADDRESS (2 * MiB)
#define BSS_DATA_SECTION_START_ADDRESS (3 * MiB)

#define UTEXT_SECTION_START_ADDRESS (4 * MiB)
#define URODATA_SECTION_START_ADDRESS (5 * MiB)
#define UBSS_UDATA_SECTION_START_ADDRESS (6 * MiB)

#define UNASSIGNED1_START_ADDRESS (7 * MiB)

#define MMIO_DEVICES_START_ADDRESS (0x3f0 * MiB)
#define UNASSIGNED2_START_ADDRESS (0x3f3 * MiB)

#define STACK_SIZE (1 * MiB)

#define USER_STACK_COUNT (32)
#define KERNEL_STACK_COUNT (6)
#define STACK_COUNT (USER_STACK_COUNT + KERNEL_STACK_COUNT)

#define KERNEL_STACK_TOP_ADDRESS (MEMORY_TOP_ADDRESS)
#define KERNEL_STACK_BOTTOM_ADDRESS (KERNEL_STACK_TOP_ADDRESS - (KERNEL_STACK_COUNT * STACK_SIZE))
#define USER_STACK_TOP_ADDRESS (KERNEL_STACK_TOP_ADDRESS - (KERNEL_STACK_COUNT * STACK_SIZE))
#define USER_STACK_BOTTOM_ADDRESS (USER_STACK_TOP_ADDRESS - (USER_STACK_COUNT * STACK_SIZE))

#define USER_SYSTEM_SP (KERNEL_STACK_TOP_ADDRESS)
#define SUPERVISOR_SP (KERNEL_STACK_TOP_ADDRESS - STACK_SIZE * 1)
#define ABORT_SP (KERNEL_STACK_TOP_ADDRESS - STACK_SIZE * 2)
#define IRQ_SP (KERNEL_STACK_TOP_ADDRESS - STACK_SIZE * 3)
#define UNDEFINED_INSTRUCTION_SP (KERNEL_STACK_TOP_ADDRESS - STACK_SIZE * 4)
#define IDLE_THREAD_SP (KERNEL_STACK_TOP_ADDRESS - STACK_SIZE * 5)

#endif