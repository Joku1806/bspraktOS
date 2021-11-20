.global get_mode_spsr
get_mode_spsr:
  mrs r1, cpsr
  cps =r0
  mrs r0, spsr
  msr cpsr_cxsf, r1
  mov pc, lr

.global get_mode_lr
get_mode_lr:
  mrs r1, cpsr
  cps =r0
  mov r0, lr
  msr cpsr_cxsf, r1
  mov pc, lr

.global get_mode_sp
get_mode_sp:
  mrs r1, cpsr
  cps =r0
  mov r0, sp
  msr cpsr_cxsf, r1
  mov pc, lr