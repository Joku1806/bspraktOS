#ifndef STACK_DEFINES_H
#define STACK_DEFINES_H

// höchste Adresse für 128MB physischen Speicher
// FIXME: vielleicht off-by-one?
#define STACK_TOP_ADDRESS 128 * 1024 * 1024

// Jeder Stack kriegt fürs erste 1kB, vielleicht ein bisschen
// übertrieben, aber den Platz haben wir im Moment noch :^)
#define STACK_SIZE 1024

// Stackpointer für die verschiedenen Interrupt-Handler.
#define USER_SYSTEM_SP STACK_TOP_ADDRESS
#define SUPERVISOR_SP STACK_TOP_ADDRESS - STACK_SIZE * 1
#define ABORT_SP STACK_TOP_ADDRESS - STACK_SIZE * 2
#define IRQ_SP STACK_TOP_ADDRESS - STACK_SIZE * 3
#define UNDEFINED_INSTRUCTION_SP STACK_TOP_ADDRESS - STACK_SIZE * 4

#endif