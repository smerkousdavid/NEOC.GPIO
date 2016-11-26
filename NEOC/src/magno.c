#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *magnoData;

int calibration[] = {0, 0, 0};

int neo_magno_set_poll(int rate) {
	FILE *polling;
	
	polling = fopen(MAGNOPOLLP, "w");

	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(polling, "%d", rate);
	fflush(polling);
	fclose(polling);

	return NEO_OK;
}

int neo_magno_init() {
	neo_check_root("Magno requires root access!");

	FILE *enabler;
	enabler = fopen(MAGNOENABLE, "w");
	if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
	fprintf(enabler, "%d", 1); //Enable accel
	fflush(enabler);
	fclose(enabler);

	magnoData = fopen(MAGNODATA, "r");
	
	if(magnoData == NULL) return NEO_UNUSABLE_ERROR;

	return neo_magno_set_poll(MAGNOPOLL);
}


int neo_magno_read(int *x, int *y, int *z) {
	if(magnoData == NULL) return NEO_UNUSABLE_ERROR;

	fflush(magnoData);
	fseek(magnoData, 0, SEEK_SET);
	if(fscanf(magnoData, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR;
	return NEO_OK;
}

int neo_magno_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_magno_read(x, y, z);

	(*x) += (calibration[0] > 0) ? -(calibration[0]) : calibration[0];
	(*y) -= (calibration[1] > 0) ? -(calibration[1]) : calibration[1];
	(*z) -= (calibration[2] > 0) ? -(calibration[2]) : calibration[2];

	return okRet;
}

int neo_magno_calibrate(int samples, int delayEach) {
	float xCal, yCal, zCal;
	int i;

	xCal = 0;
	yCal = 0;
	zCal = 0;
	for(i = 0; i < samples; i++) {
		int x, y, z;

		if(neo_magno_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		xCal += ((float)x);
		yCal += ((float)y);
		zCal += ((float)z);

		usleep(1000 * delayEach);
	}

	calibration[0] = (int)(xCal / ((float)samples));
	calibration[1] = (int)(yCal / ((float)samples));
	calibration[2] = (int)(zCal / ((float)samples));

	return NEO_OK;
}

int neo_magno_free() {
	FILE* enabler;
	enabler = fopen(MAGNOENABLE, "w");

	if(enabler == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(enabler, "%d", 1);
	fflush(enabler);
	fclose(enabler);

	return NEO_OK;
}


/*int main() {
	int x, y, z;
	printf("INIT: %d", neo_magno_init());
	//neo_gyro_calibrate(25, 150);
	x = 0;
	y = 0;
	z = 0;
	while(1) {
		neo_magno_read(&x, &y, &z);	
		printf("X: %d Y: %d Z: %d\n", x, y, z);
		usleep(1000*20);
	}
	neo_magno_free();
}*/

