#include <neo.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

void intfunction(int pin, int data) {
	std::cout << "P: " << pin << " D: " << data << std::endl;
}

void intmorefunction(int pin, int data) {
	std::cout << "P: " << pin << " D: " << data << std::endl;
}

int main() {
	neo::Gpio gp(12);
	gp.attachInterrupt("falling", &intfunction);
	
	neo::Gpio::attachInterrupt(13, "both", [](int pin, int data) {
		std::cout << "lambda P: " << pin << " D: " << data << std::endl;
	});

	while(1) {
	
	}

	return 0;
}
