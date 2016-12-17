#include <neo.h>
#include <chrono>
#include <thread>


int main() {
	neo::BuiltinLED led;

	for(int parid = 0; parid < 10; parid++) {
		led.on();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		led.off();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}


	return 0;
}
