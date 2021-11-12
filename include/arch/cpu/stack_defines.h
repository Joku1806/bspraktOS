// höchste Adresse für 128MB physischen Speicher
// FIXME: vielleicht off-by-one?
#define STACK_TOP_ADDRESS 128 * 1024 * 1024

// Jeder Stack kriegt fürs erste 1MB, vielleicht ein bisschen
// übertrieben, aber den Platz haben wir im Moment noch :^)
#define STACK_SIZE 1024 * 1024

// Stackpointer für die verschiedenen Interrupt-Handler.
#define RESET_INTERRUPT_HANDLER_SP STACK_TOP_ADDRESS
#define UNDEFINED_INSTRUCTION_INTERRUPT_HANDLER_SP                             \
  STACK_TOP_ADDRESS - STACK_SIZE * 1
#define SOFTWARE_INTERRUPT_HANDLER_SP STACK_TOP_ADDRESS - STACK_SIZE * 2
#define PREFETCH_ABORT_INTERRUPT_HANDLER_SP STACK_TOP_ADDRESS - STACK_SIZE * 3
#define DATA_ABORT_INTERRUPT_HANDLER_SP STACK_TOP_ADDRESS - STACK_SIZE * 4
#define IRQ_INTERRUPT_HANDLER_SP STACK_TOP_ADDRESS - STACK_SIZE * 5
#define ENTRY_SP STACK_TOP_ADDRESS - STACK_SIZE * 6