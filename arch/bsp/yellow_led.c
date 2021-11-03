#include <arch/bsp/yellow_led.h>
#include <stdint.h>

#define GPIO_BASE (0x7E200000 - 0x3F000000)
#define FIRST_GREEN_LED 4
#define FIRST_YELLOW_LED 5
#define ONLY_RED_LED 6
#define SECOND_YELLOW_LED 7
#define SECOND_GREEN_LED 8
#define GPF_BITS 3

enum gpio_func {
  gpio_input = 0x0,  // GPIO Pin is an input
  gpio_output = 0x1, // GPIO Pin is an output
};

struct gpio {
  uint32_t func[6];
  uint32_t unused0;
  uint32_t set[2];
  uint32_t unused1;
  uint32_t clr[2];
};

static volatile struct gpio *const gpio_port = (struct gpio *)GPIO_BASE;

void yellow_on(void) {
  /* Initialisieren */
  gpio_port->func[0] = gpio_output << (FIRST_YELLOW_LED * GPF_BITS);

  /* Anschalten */
  gpio_port->set[0] = 1 << FIRST_YELLOW_LED;
}
