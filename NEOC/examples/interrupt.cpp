#include <neo.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

int main() {
	neo::Gpio gpio(13);
	
	gpio.attachInterrupt("falling",
	[] (int pin, int data) {
		std::cout << "P: " << pin << " D: " << data << std::endl;
	});

	while(1) {
	
	}

	return 0;
}
