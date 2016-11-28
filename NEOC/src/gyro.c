#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *neo_gyro_data;

int neo_gyro_calibration[] = {0, 0, 0};

unsigned char neo_gyro_freed = 2;

int neo_gyro_set_poll(int rate) {
	FILE *polling;
	
	polling = fopen(GYROPOLLP, "w");

	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(polling, "%d", rate);
	fflush(polling);
	fclose(polling);
	return NEO_OK;
}

int neo_gyro_init() {
	if(neo_gyro_freed == 2) {
		neo_check_root("Gyro requires root access!");
	
		FILE *enabler;
		enabler = fopen(GYROENABLE, "w");
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
		
		fprintf(enabler, "%d", 1); //Enable accel
		fflush(enabler);
		fclose(enabler);
	
		neo_gyro_data = fopen(neo_gyro_data, "r");
		
		if(neo_gyro_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_gyro_freed = 0;
	}
	return neo_gyro_set_poll(GYROPOLL);
}


int neo_gyro_read(int *x, int *y, int *z) {
	if(neo_gyro_data == NULL) return NEO_UNUSABLE_ERROR;

	fflush(neo_gyro_data);
	fseek(neo_gyro_data, 0, SEEK_SET);
	if(fscanf(neo_gyro_data, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR;
	return NEO_OK;
}

int neo_gyro_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_gyro_read(x, y, z);

	(*x) += (neo_gyro_calibration[0] > 0) ? -(neo_gyro_calibration[0]) : neo_gyro_calibration[0];
	(*y) -= (neo_gyro_calibration[1] > 0) ? -(neo_gyro_calibration[1]) : neo_gyro_calibration[1];
	(*z) -= (neo_gyro_calibration[2] > 0) ? -(neo_gyro_calibration[2]) : neo_gyro_calibration[2];

	return okRet;
}

int neo_gyro_calibrate(int samples, int delayEach) {
	float xCal, yCal, zCal;
	int i;

	xCal = 0;
	yCal = 0;
	zCal = 0;
	for(i = 0; i < samples; i++) {
		int x, y, z;

		if(neo_gyro_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		xCal += ((float)x);
		yCal += ((float)y);
		zCal += ((float)z);

		usleep(1000 * delayEach);
	}

	neo_gyro_calibration[0] = (int)(xCal / ((float)samples));
	neo_gyro_calibration[1] = (int)(yCal / ((float)samples));
	neo_gyro_calibration[2] = (int)(zCal / ((float)samples));

	return NEO_OK;
}

int neo_gyro_free() {
	if(neo_gyro_freed == 0) {
		FILE* enabler;
		enabler = fopen(GYROENABLE, "w");
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 1);
		fflush(enabler);
		fclose(enabler);
		neo_gyro_freed = 2;
	}
	return NEO_OK;
}


/*int main() {
	int x, y, z;
	neo_gyro_init();
	//neo_gyro_calibrate(25, 150);
	while(1) {
		neo_gyro_read(&x, &y, &z);	
		printf("X: %d Y: %d Z: %d\n", x, y, z);
		usleep(1000*2000);
	}
	neo_gyro_free();
}*/
