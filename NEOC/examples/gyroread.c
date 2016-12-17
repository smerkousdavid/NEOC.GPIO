#include <neo.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>


int main() {
	neo_gyro_init();
	
	printf("Starting...\n");

	neo_gyro_set_poll(300);

	printf("Calibrating...\n");

	neo_gyro_calibrate(20, 300);

	printf("Calibrated\n");


	while(1) {
		int x, y, z;
		neo_gyro_read_calibrated(&x, &y, &z);

		printf("X: %d, Y: %d, Z: %d\n", x, y, z);
	
		usleep(1000 * 300);
	}


	return 0;
}
