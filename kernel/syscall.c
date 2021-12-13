#include <kernel/syscall.h>
#include <stdint.h>

uint32_t get_syscall_no(uint32_t svc_instruction_address) {
  return *(uint32_t *)svc_instruction_address & 0x00ffffff;
}

bool is_syscall(uint32_t instruction_address) {
  return (*(uint32_t *)instruction_address & 0xff000000) == 0xef000000;
}

void sys$exit() { asm volatile("svc %0 \n\t" ::"I"(SYSCALL_EXIT_NO)); }