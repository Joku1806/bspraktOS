#ifndef STACK_DEFINES_H
#define STACK_DEFINES_H

// höchste Adresse für 128MB physischen Speicher
#define STACK_TOP_ADDRESS 128 * 1024 * 1024 - 8

// Jeder Stack kriegt fürs erste 1kB
#define STACK_SIZE 1024 * 4

// Stackpointer für die verschiedenen Interrupt-Handler.
#define USER_SYSTEM_SP STACK_TOP_ADDRESS
#define SUPERVISOR_SP STACK_TOP_ADDRESS - STACK_SIZE * 1
#define ABORT_SP STACK_TOP_ADDRESS - STACK_SIZE * 2
#define IRQ_SP STACK_TOP_ADDRESS - STACK_SIZE * 3
#define UNDEFINED_INSTRUCTION_SP STACK_TOP_ADDRESS - STACK_SIZE * 4

#define THREAD_SP_BASE STACK_TOP_ADDRESS - STACK_SIZE * 5

#endif