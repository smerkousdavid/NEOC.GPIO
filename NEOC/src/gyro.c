#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *gyroData;

int gyro_calibration[] = {0, 0, 0};

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
	neo_check_root("Gyro requires root access!");

	FILE *enabler;
	enabler = fopen(GYROENABLE, "w");

	if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
	fprintf(enabler, "%d", 1); //Enable accel
	fflush(enabler);
	fclose(enabler);

	gyroData = fopen(GYRODATA, "r");
	
	if(gyroData == NULL) return NEO_UNUSABLE_ERROR;

	return neo_gyro_set_poll(GYROPOLL);
}


int neo_gyro_read(int *x, int *y, int *z) {
	if(gyroData == NULL) return NEO_UNUSABLE_ERROR;

	fflush(gyroData);
	fseek(gyroData, 0, SEEK_SET);
	if(fscanf(gyroData, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR;
	return NEO_OK;
}

int neo_gyro_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_gyro_read(x, y, z);

	(*x) += (gyro_calibration[0] > 0) ? -(gyro_calibration[0]) : gyro_calibration[0];
	(*y) -= (gyro_calibration[1] > 0) ? -(gyro_calibration[1]) : gyro_calibration[1];
	(*z) -= (gyro_calibration[2] > 0) ? -(gyro_calibration[2]) : gyro_calibration[2];

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

	gyro_calibration[0] = (int)(xCal / ((float)samples));
	gyro_calibration[1] = (int)(yCal / ((float)samples));
	gyro_calibration[2] = (int)(zCal / ((float)samples));

	return NEO_OK;
}

int neo_gyro_free() {
	FILE* enabler;
	enabler = fopen(GYROENABLE, "w");

	if(enabler == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(enabler, "%d", 1);
	fflush(enabler);
	fclose(enabler);

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
