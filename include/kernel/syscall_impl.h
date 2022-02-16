#ifndef SYSCALL_IMPL
#define SYSCALL_IMPL

#include <arch/cpu/registers.h>
#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_READ_CHARACTER_NO 1
#define SYSCALL_OUTPUT_CHARACTER_NO 2
#define SYSCALL_CREATE_PROCESS_NO 3
#define SYSCALL_CREATE_THREAD_NO 4
#define SYSCALL_STALL_THREAD_NO 5
#define SYSCALL_EXIT_THREAD_NO 6
#define SYSCALL_GET_TIME_NO 7

bool is_valid_syscall(void *instruction_address);
uint32_t get_syscall_no(void *svc_instruction_address);

int dispatch_syscall(registers *regs, uint32_t syscall_no);

#endif