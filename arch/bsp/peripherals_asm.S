#include <arch/bsp/peripherals.h>

.global enable_peripheral_interrupts
.type enable_peripheral_interrupts, %function
enable_peripheral_interrupts:
  @ Interrupts aktivieren (nur aktueller Modus, sollte System sein!)
  cpsie i
  @ sysimer IRQ interrupts aktivieren (wir benutzen C1, deswegen TIMER1)
  ldr r1, =(INTERRUPT_PERIPHERALS_BASE + ENABLE_IRQS_1_OFFSET)
  ldr r0, [r1]
  orr r0, r0, #TIMER1_ENABLE_FLAG
  str r0, [r1]

  @ FIXME: zweiter Timer für sys$stall_thread()

  ldr r1, =(INTERRUPT_PERIPHERALS_BASE + ENABLE_IRQS_2_OFFSET)
  ldr r0, [r1]
  orr r0, r0, #UART_ENABLE_FLAG
  str r0, [r1]

  bx lr