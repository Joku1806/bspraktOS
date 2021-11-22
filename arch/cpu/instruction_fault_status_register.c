#include <arch/cpu/instruction_fault_status_register.h>
#include <lib/assertions.h>

const char *get_prefetch_abort_error_type(uint32_t ifsr) {
  switch ((ifsr & ifsr_FS4) << 4 | (ifsr & ifsr_FS0_3)) {
    case PC_alignment_fault:
      return "PC Alignment Fault";
    case debug_exception:
      return "Debug Exception";
    case access_flag_fault_level_1:
      return "Access Flag Fault (level 1)";
    case translation_fault_level_1:
      return "Translation Fault (level 1)";
    case access_flag_fault_level_2:
      return "Access Flag Fault (level 2)";
    case translation_fault_level_2:
      return "Translation Fault (level 2)";
    case synchronous_external_abort_not_on_translation_table_walk:
      return "Synchronous External Abort (not on translation table walk)";
    case domain_fault_level_1:
      return "Domain Fault (level 1)";
    case domain_fault_level_2:
      return "Domain Fault (level 2)";
    case synchronous_external_abort_on_translation_table_walk_level_1:
      return "Synchronous External Abort (on translation table walk, level 1)";
    case permission_fault_level_1:
      return "Permission Fault (level 1)";
    case synchronous_external_abort_on_translation_table_walk_level_2:
      return "Synchronous External Abort (on translation table walk, level 2)";
    case permission_fault_level_2:
      return "Permission Fault (level 2)";
    case TLB_conflict_abort:
      return "TLB Conflict Abort";
    case IMPLEMENTATION_DEFINED_fault_lockdown_fault:
      return "IMPLEMENTATION DEFINED Fault (lockdown fault)";
    case synchronous_parity_or_ECC_error_on_memory_access_not_on_translation_table_walk:
      return "Synchronous Parity Or ECC Error (on memory access, not on "
             "translation table walk)";
    case synchronous_parity_or_ECC_error_on_translation_table_walk_level_1:
      return "Synchronous Parity Or ECC Error (on translation table walk, "
             "level 1)";
    case synchronous_parity_or_ECC_error_on_translation_table_walk_level_2:
      return "Synchronous Parity Or ECC Error (on translation table walk, "
             "level 2)";
  }

  VERIFY_NOT_REACHED();
}