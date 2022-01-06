#include <arch/cpu/registers.h>
#include <kernel/syscall.h>
#include <lib/math.h>
#include <lib/string.h>
#include <stdint.h>

uint32_t get_syscall_no(void *svc_instruction_address) {
  return *(uint32_t *)svc_instruction_address & 0x00ffffff;
}

bool is_syscall(void *instruction_address) {
  return (*(uint32_t *)instruction_address & 0xff000000) == 0xef000000;
}

char sys$read_character() {
  uint32_t sp = get_current_sp();
  unsigned size = sizeof(char);
  unsigned aligned_size = align8(size);
  sp -= aligned_size;
  set_current_sp(sp);

  asm volatile("svc %0 \n\t" ::"I"(SYSCALL_READ_CHARACTER_NO));

  char read = *(char *)sp;
  sp += aligned_size;
  set_current_sp(sp);

  return read;
}

void sys$output_character(char ch) {
  uint32_t sp = get_current_sp();
  unsigned size = sizeof(char);
  unsigned aligned_size = align8(size);
  sp -= aligned_size;
  memcpy((void *)sp, &ch, size);
  set_current_sp(sp);

  asm volatile("svc %0 \n\t" ::"I"(SYSCALL_OUTPUT_CHARACTER_NO));

  sp += aligned_size;
  set_current_sp(sp);
}

void sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size) {
  uint32_t sp = get_current_sp();
  unsigned size = sizeof(void *) + sizeof(void *) + sizeof(unsigned int);
  unsigned aligned_size = align8(size);
  sp -= aligned_size;
  memcpy((void *)sp, &func, sizeof(void *));
  memcpy((void *)(sp + sizeof(void *)), &args, sizeof(void *));
  memcpy((void *)(sp + sizeof(void *) + sizeof(void *)), &args_size, sizeof(unsigned int));
  set_current_sp(sp);

  asm volatile("svc %0 \n\t" ::"I"(SYSCALL_CREATE_THREAD_NO));

  sp += aligned_size;
  set_current_sp(sp);
}

void sys$stall_thread(unsigned ms) {
  uint32_t sp = get_current_sp();
  unsigned size = sizeof(unsigned);
  unsigned aligned_size = align8(size);
  sp -= aligned_size;
  memcpy((void *)sp, &ms, size);
  set_current_sp(sp);

  asm volatile("svc %0 \n\t" ::"I"(SYSCALL_STALL_THREAD_NO));

  sp += aligned_size;
  set_current_sp(sp);
}

void sys$exit_thread() {
  asm volatile("svc %0 \n\t" ::"I"(SYSCALL_EXIT_THREAD_NO));
}