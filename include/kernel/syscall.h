#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_EXIT_NO 1

bool is_syscall(uint32_t instruction_address);
uint32_t get_syscall_no(uint32_t svc_instruction_address);

void sys$exit();

#endif