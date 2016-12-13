#include <neo.h>
#include <chrono>
#include <thread>


using namespace std;

int main() {
	neo::FakePWM pwm(13);
	pwm.setPeriod(20408);

	while(1) {
		//Fade up to 255
		for(int i = 0; i < 255; i++) {
			pwm.write(i); //Write up pwm value
			this_thread::sleep_for(chrono::milliseconds(10));
		}

		//Fade down back to 0
		for(int d = 255; d > 0; d--) {
			pwm.write(d); //Write down pwm value
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		this_thread::sleep_for(chrono::milliseconds(400));
	}

	return 0;
}
