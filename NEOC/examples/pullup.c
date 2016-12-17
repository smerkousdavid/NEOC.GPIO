#include <neo.h>
#include <unistd.h>
#include <time.h>


int main() {
	
	neo_gpio_init(); //Start gpio

	neo_gpio_pin_mode(12, INPUT); //Set pin 12 to input

	neo_gpio_digital_write(12, HIGH); //Use Pull up resistor on pin 12

	while(1) {
		int ret = neo_gpio_digital_read(12);

		if(ret < NEO_OK) {
			printf("Failed reading from pulled up pin 12\n"); //Read from pin 12
		}

		printf("P12: %d\n", ret);
		sleep(1); //Wait one second
	}


	return 0;
}
