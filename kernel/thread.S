load_thread_context:
  add r0, r0, #4 @ skip node*
  str r1, [r0, #16 * 4]
  msr cpsr, r1
  ldmfd r0, {r0-r15}