#include <arch/bsp/interrupt_peripherals.h>

#define SAVED_REGS_COUNT 16

.global _setup_ivt
_setup_ivt:
  @ VBAR mit _ivt initialisieren
  ldr r0, =_ivt
  mcr p15, 0, r0, c12, c0, 0
  @ FIXME: sollte der kernel ganz am Ende von init machen
  @ Interrupts aktivieren (nur aktueller Modus, sollte System sein!)
  cpsie i
  @ sysimer IRQ interrupts aktivieren (wir benutzen C1, deswegen TIMER1)
  ldr r1, =(INTERRUPT_PERIPHERALS_BASE + ENABLE_IRQS_1_OFFSET)
  ldr r0, [r1]
  orr r0, r0, #TIMER1_ENABLE_FLAG
  str r0, [r1]

  ldr r1, =(INTERRUPT_PERIPHERALS_BASE + ENABLE_IRQS_2_OFFSET)
  ldr r0, [r1]
  orr r0, r0, #UART_ENABLE_FLAG
  str r0, [r1]

  bx lr

.global _ivt
.balign 64
_ivt:
  ldr pc, =reset_interrupt_handler
  ldr pc, =_undefined_instruction
  ldr pc, =_software_interrupt
  ldr pc, =_prefetch_abort
  ldr pc, =_data_abort
  b halt_cpu @ wird nicht benutzt, sollte also niemals passieren
  ldr pc, =_irq

_undefined_instruction:
  stmfd sp, {r0-r15}
  sub sp, sp, #(SAVED_REGS_COUNT * 4)
  mov r0, sp
  bl undefined_instruction_interrupt_handler
  ldmfd sp, {r0-r14}
  movs pc, lr

_software_interrupt:
  stmfd sp, {r0-r15}
  sub sp, sp, #(SAVED_REGS_COUNT * 4)
  mov r0, sp
  bl software_interrupt_handler
  ldmfd sp, {r0-r14}
  movs pc, lr

_prefetch_abort:
  stmfd sp, {r0-r15}
  sub sp, sp, #(SAVED_REGS_COUNT * 4)
  mov r0, sp
  bl prefetch_abort_interrupt_handler
  ldmfd sp, {r0-r14}
  movs pc, lr

_data_abort:
  sub lr, lr, #4
  stmfd sp, {r0-r15}
  sub sp, sp, #(SAVED_REGS_COUNT * 4)
  mov r0, sp
  bl data_abort_interrupt_handler
  ldmfd sp, {r0-r14}
  movs pc, lr

_irq:
  sub lr, lr, #4
  stmfd sp, {r0-r15}
  sub sp, sp, #(SAVED_REGS_COUNT * 4)
  mov r0, sp
  bl irq_interrupt_handler
  ldmfd sp, {r0-r14}
  movs pc, lr
