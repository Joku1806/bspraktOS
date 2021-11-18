.global get_mode_spsr
get_mode_spsr:
  mrs r1, cpsr
  cps =r0
  mrs r0, spsr
  msr cpsr, r1
  mov pc, lr

.global get_mode_lr
get_mode_lr:
  mrs r1, cpsr
  cps =r0
  mov r0, lr
  msr cpsr, r1
  mov pc, lr

.global get_mode_pc
get_mode_pc:
  mrs r1, cpsr
  cps =r0
  mov r0, pc
  msr cpsr, r1
  mov pc, lr