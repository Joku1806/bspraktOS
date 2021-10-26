#include <arch/bsp/yellow_led.h>

volatile unsigned int counter = 0;

void increment_counter() {
	counter++;
}


void start_kernel(){

	yellow_on();


	// Endless counter
	for (;;) {
		increment_counter();
	}


}
