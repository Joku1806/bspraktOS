#define LOG_LEVEL WARNING_LEVEL
#define LOG_LABEL "DFSR"

#include <arch/cpu/dfsr.h>
// FIXME: Sollte nicht VERIFY_NOT_REACHED() benutzen, sondern stattdessen
// -K_EINVAL zurückgeben.
#include <kernel/lib/kassertions.h>

const char *get_data_abort_error_type(uint32_t dfsr) {
  switch (dfsr & dfsr_status) {
    case dfsr_alignment_fault:
      return "Alignment Fault";
    case dfsr_instruction_cache_maintenance_fault:
      return "Instruction Cache Maintenance Fault";
    case dfsr_synchronous_external_abort_1st_level_translation:
      return "Synchronous External Abort (1st level translation)";
    case dfsr_synchronous_external_abort_2nd_level_translation:
      return "Synchronous External Abort (2nd level translation)";
    case dfsr_section_translation_fault:
      return "Section Translation Fault";
    case dfsr_page_translation_fault:
      return "Page Translation Fault";
    case dfsr_section_access_flag_fault:
      return "Section Access Flag Fault";
    case dfsr_page_access_flag_fault:
      return "Page Access Flag Fault";
    case dfsr_section_domain_fault:
      return "Section Domain Fault";
    case dfsr_page_domain_fault:
      return "Page Domain Fault";
    case dfsr_section_permission_fault:
      return "Section Permission Fault";
    case dfsr_page_permission_fault:
      return "Page Permission Fault";
    case dfsr_nontranslation_synchronous_external_abort:
      return "Synchronous External Abort (nontranslation)";
    case dfsr_debug_event:
      return "Debug Event";
  }

  // dfsr_asynchronous_external_abort nicht gecheckt, weil der Wert =
  // dfsr_page_access_flag_fault ist. bessere Ausgabe mit FS und ExT kann später
  // gemacht werden...
  VERIFY_NOT_REACHED();
}

const char *get_data_abort_access_type(uint32_t dfsr) {
  switch (dfsr & dfsr_WnR) {
    case WnR_read:
      return "lesend";
    case WnR_write:
      return "schreibend";
  }

  VERIFY_NOT_REACHED();
}