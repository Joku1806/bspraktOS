#ifndef SYSCALL_H
#define SYSCALL_H

#ifdef __ASSEMBLER__

#define SYSCALL_READ_CHARACTER_NO 1
#define SYSCALL_OUTPUT_CHARACTER_NO 2
#define SYSCALL_CREATE_THREAD_NO 3
#define SYSCALL_STALL_THREAD_NO 4
#define SYSCALL_EXIT_THREAD_NO 5
#define SYSCALL_GET_TIME_NO 6

#else

#include <stdbool.h>
#include <stdint.h>

#define SYSCALL_READ_CHARACTER_NO 1
#define SYSCALL_OUTPUT_CHARACTER_NO 2
#define SYSCALL_CREATE_THREAD_NO 3
#define SYSCALL_STALL_THREAD_NO 4
#define SYSCALL_EXIT_THREAD_NO 5
#define SYSCALL_GET_TIME_NO 6

char sys$read_character();
void sys$output_character(char ch);
int sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size);
void sys$stall_thread(unsigned ms);
_Noreturn void sys$exit_thread();
uint32_t sys$get_time();

#endif

#endif