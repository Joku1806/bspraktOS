.global _setup_ivt
_setup_ivt:
  @ VBAR mit _ivt initialisieren
  ldr r0, =_ivt
  mcr p15, 0, r0, c12, c0, 0
  @ Interrupts aktivieren
  cpsie i
  bx lr

.global _ivt
.balign 64
_ivt:
  b reset_interrupt_handler
  ldr pc, =_undefined_instruction
  ldr pc, =_software_interrupt
  ldr pc, =_prefetch_abort
  ldr pc, =_data_abort
  b halt_cpu @ nicht benutzt, sollte also niemals passieren
  ldr pc, =_irq

_undefined_instruction:
  @ TODO: save regs
  bl undefined_instruction_interrupt_handler
  @ TODO: restore regs

_software_interrupt:
  @ TODO: save regs
  bl software_interrupt_handler
  @ TODO: restore regs

_prefetch_abort:
  @ TODO: save regs
  ldr sp, =PREFETCH_ABORT_INTERRUPT_HANDLER_SP
  bl prefetch_abort_interrupt_handler
  @ TODO: restore regs

_data_abort:
  @ TODO: save regs
  bl data_abort_interrupt_handler
  @ TODO: restore regs

_irq:
  @ TODO: save regs
  bl irq_interrupt_handler
  @ TODO: restore regs
