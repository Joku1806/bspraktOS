# BSprakt

Syscall documentation:

	1: sys$read_character()
usage: reads a character and returns it
arguments: none
return type: char

	2: sys$output_character(char ch)
usage: prints a given character
arguments: char
return type: void

	3: sys$create_thread(void (*func)(void *), const void *args, unsigned int args_size) __attribute__((weak))
usage: creates a new thread with the given arguments
arguments: function, const void *args, unsigned int
return type: void
*CAUTION*: kann -EBUSY zurückgeben, wenn zurzeit keine threads verfügbar sind

	4: sys$stall_thread(unsigned ms)
usage: stalls current thread for a given ammount of milliseconds
arguments: unsigned int
return type: void

	5: sys$exit_thread()
usage: exits the current thread
arguments: none
return type: void