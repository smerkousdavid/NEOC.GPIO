#include <neo.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <unistd.h>

//OPTIONAL SCALED VALUE
//#define SCALEANALOG

using namespace neo;


int main() {
	neo_disable_m4(); //Make sure you disblae the m4 core (this doesn't need to be ran everytime)

	//int ret; //Variable to hold the return
	//ret = neo_analog_init();

	std::cout << "init: " << Analog::init() << std::endl;
	
	neo::Analog analog(0);

	printf("ZERO: %f", analog.read());

	//if(ret != NEO_OK) {
	//	printf("FAILED INITIAL SETUP FOR ANALOG!\n");
	//	return 1;
	//}

	int i, id;
	
	for(i = 0; i < 3; i++) {
		printf("Trial %d\n", i);
		for(id = 0; id <= 5; id++) {
			printf("ANALOG(A%d): %.2f out of 4095\n", id, Analog::read(id));
		}
		printf("\n\n");
		usleep(1000 * 1000);
	}
	std::cout << "free: " << Analog::free() << std::endl;
	printf("Done reading analog!\n");
	return 0;
}
