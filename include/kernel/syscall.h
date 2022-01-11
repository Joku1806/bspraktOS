#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_READ_CHARACTER_NO 1
#define SYSCALL_OUTPUT_CHARACTER_NO 2
#define SYSCALL_CREATE_THREAD_NO 3
#define SYSCALL_STALL_THREAD_NO 4
#define SYSCALL_EXIT_THREAD_NO 5

bool is_syscall(void *instruction_address);
uint32_t get_syscall_no(void *svc_instruction_address);

char sys$read_character() __attribute__((weak));
void sys$output_character(char ch) __attribute__((weak));
void sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size) __attribute__((weak));
void sys$stall_thread(unsigned ms) __attribute__((weak));
void sys$exit_thread() __attribute__((weak));

#endif