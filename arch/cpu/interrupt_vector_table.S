#define SAVED_REGS_COUNT 18

.global _setup_ivt
_setup_ivt:
  @ VBAR mit _ivt initialisieren
  ldr r0, =_ivt
  mcr p15, 0, r0, c12, c0, 0
  @ Interrupts aktivieren (nur aktueller Modus, sollte System sein!)
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
  b halt_cpu @ fiq wird nicht benutzt, sollte also niemals passieren
  ldr pc, =_irq

_undefined_instruction:
  stmfd sp, {r0-r15}
  @ hässliche Lösung, vllt zwei separate defines?
  sub sp, sp, #((SAVED_REGS_COUNT - 2) * 4)
  mrs r0, cpsr
  mrs r1, spsr
  stmfd sp, {r0,r1}
  sub sp, sp, #((SAVED_REGS_COUNT - 16) * 4)
  mov r0, sp
  bl undefined_instruction_interrupt_handler
  ldmfd sp, {r0, r1}
  msr cpsr_cxsf, r0
  msr spsr_cxsf, r1
  ldmfd sp, {r0-r14}
  movs pc, lr

_software_interrupt:
  stmfd sp, {r0-r15}
  @ hässliche Lösung, vllt zwei separate defines?
  sub sp, sp, #((SAVED_REGS_COUNT - 2) * 4)
  mrs r0, cpsr
  mrs r1, spsr
  stmfd sp, {r0,r1}
  sub sp, sp, #((SAVED_REGS_COUNT - 16) * 4)
  mov r0, sp
  bl software_interrupt_handler
  ldmfd sp, {r0, r1}
  msr cpsr_cxsf, r0
  msr spsr_cxsf, r1
  ldmfd sp, {r0-r14}
  movs pc, lr

_prefetch_abort:
  stmfd sp, {r0-r15}
  @ hässliche Lösung, vllt zwei separate defines?
  sub sp, sp, #((SAVED_REGS_COUNT - 2) * 4)
  mrs r0, cpsr
  mrs r1, spsr
  stmfd sp, {r0,r1}
  sub sp, sp, #((SAVED_REGS_COUNT - 16) * 4)
  mov r0, sp
  bl prefetch_abort_interrupt_handler
  ldmfd sp, {r0, r1}
  msr cpsr_cxsf, r0
  msr spsr_cxsf, r1
  ldmfd sp, {r0-r14}
  movs pc, lr

_data_abort:
  stmfd sp, {r0-r15}
  @ hässliche Lösung, vllt zwei separate defines?
  sub sp, sp, #((SAVED_REGS_COUNT - 2) * 4)
  mrs r0, cpsr
  mrs r1, spsr
  stmfd sp, {r0,r1}
  sub sp, sp, #((SAVED_REGS_COUNT - 16) * 4)
  mov r0, sp
  bl data_abort_interrupt_handler
  ldmfd sp, {r0, r1}
  msr cpsr_cxsf, r0
  msr spsr_cxsf, r1
  ldmfd sp, {r0-r14}
  movs pc, lr

_irq:
  stmfd sp, {r0-r15}
  @ hässliche Lösung, vllt zwei separate defines?
  sub sp, sp, #((SAVED_REGS_COUNT - 2) * 4)
  mrs r0, cpsr
  mrs r1, spsr
  stmfd sp, {r0,r1}
  sub sp, sp, #((SAVED_REGS_COUNT - 16) * 4)
  mov r0, sp
  bl irq_interrupt_handler
  ldmfd sp, {r0, r1}
  msr cpsr_cxsf, r0
  msr spsr_cxsf, r1
  ldmfd sp, {r0-r14}
  movs pc, lr
