#include <arch/cpu/psr.h>
#include <arch/bsp/stack_defines.h>

initialize_thread_state:
  msr cpsr, MODE_USR
  mov r3, THREAD_SP_BASE
  mul r0, r0, STACK_SIZE
  sub r3, r3, r0  @r0 ist der Thread index
  mov sp, r3
  mov lr, r2  @r2 ist die Endfunktion
  mov pc, r1  @r1 ist die Startfunktion

reload_thread_state:
  add r0, r0, #4
  str r1, [r0, #16 * 4]
  msr cpsr, r1
  ldmfd r0, {r0-r15}