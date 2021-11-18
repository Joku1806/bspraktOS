#define NUMBER_OF_MODES 5

#ifdef __ASSEMBLER__

#define MODE_IRQ 0x12
#define MODE_SUPERVISOR 0x13
#define MODE_ABORT 0x17
#define MODE_UNDEFINED 0x1b
#define MODE_SYSTEM 0x1f

#else

typedef enum {
  m_irq = 0x12,
  m_supervisor = 0x13,
  m_abort = 0x17,
  m_undefined = 0x1b,
  m_system = 0x1f
} cpu_mode;

#endif