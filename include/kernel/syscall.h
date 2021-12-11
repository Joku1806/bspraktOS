#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYSCALL_EXIT_NO 1

uint32_t get_syscall_no(uint32_t svc_instruction_address);

void sys$exit();

#endif