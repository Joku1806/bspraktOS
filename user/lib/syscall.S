#include <user/lib/syscall.h>

.global sys$read_character
sys$read_character:
  svc #SYSCALL_READ_CHARACTER_NO
  bx lr

.global sys$output_character
sys$output_character:
  svc #SYSCALL_OUTPUT_CHARACTER_NO
  bx lr

.global sys$create_thread
sys$create_thread:
  svc #SYSCALL_CREATE_THREAD_NO
  bx lr

.global sys$stall_thread
sys$stall_thread:
  svc #SYSCALL_STALL_THREAD_NO
  bx lr

.global sys$exit_thread
sys$exit_thread:
  svc #SYSCALL_EXIT_THREAD_NO
  bx lr

.global sys$get_time
sys$get_time:
  svc #SYSCALL_GET_TIME_NO
  bx lr