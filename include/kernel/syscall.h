#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_EXIT_NO 1

bool is_syscall(void *instruction_address);
uint32_t get_syscall_no(void *svc_instruction_address);

void sys$exit();

#endif