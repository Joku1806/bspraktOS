#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_READ_CHARACTER_NO 1
#define SYSCALL_OUTPUT_CHARACTER_NO 2
#define SYSCALL_CREATE_PROCESS_NO 3
#define SYSCALL_CREATE_THREAD_NO 4
#define SYSCALL_STALL_THREAD_NO 5
#define SYSCALL_EXIT_THREAD_NO 6
#define SYSCALL_GET_TIME_NO 7
#define SYSCALL_GET_THREAD_ID_NO 8

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

char sys$read_character();
void sys$output_character(char ch);
int sys$create_process(void (*func)(void *), const void *args, unsigned int args_size);
int sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size);
int sys$stall_thread(unsigned ms);
_Noreturn void sys$exit_thread();
uint32_t sys$get_time();
size_t sys$get_thread_id();

#endif

#endif