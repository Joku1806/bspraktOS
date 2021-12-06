reload_thread_state:
  add r0, r0, #4
  str r1, [r0, #16 * 4]
  msr cpsr, r1
  ldmfd r0, {r0-r15}