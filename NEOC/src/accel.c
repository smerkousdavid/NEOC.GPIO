#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *neo_accel_data;

int neo_accel_calibration[] = {0, 0, 0};

unsigned char neo_accel_freed = 2;

int neo_accel_set_poll(int rate) {
	FILE *polling;
	
	polling = fopen(ACCELPOLLP, "w");

	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(polling, "%d", rate);
	fflush(polling);
	fclose(polling);

	return NEO_OK;
}

int neo_accel_init() {
	if(neo_accel_freed == 2) {
		neo_check_root("Accelerometer requires root access!");
	
		FILE *enabler;
		enabler = fopen(ACCELENABLE, "w");
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 1); //Enable accel
		fflush(enabler);
		fclose(enabler);
	
		neo_accel_data = fopen(neo_accel_data, "r");
		
		if(neo_accel_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_accel_freed = 0;
	}

	return neo_accel_set_poll(ACCELPOLL);
}


int neo_accel_read(int *x, int *y, int *z) {
	if(neo_accel_data == NULL) return NEO_UNUSABLE_ERROR;

	fflush(neo_accel_data);
	fseek(neo_accel_data, 0, SEEK_SET);
	if(fscanf(neo_accel_data, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR;
	return NEO_OK;
}

int neo_accel_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_accel_read(x, y, z);

	(*x) += (neo_accel_calibration[0] > 0) ? -(neo_accel_calibration[0]) : neo_accel_calibration[0];
	(*y) -= (neo_accel_calibration[1] > 0) ? -(neo_accel_calibration[1]) : neo_accel_calibration[1];
	(*z) -= (neo_accel_calibration[2] > 0) ? -(neo_accel_calibration[2]) : neo_accel_calibration[2];

	return okRet;
}

int neo_accel_calibrate(int samples, int delayEach) {
	float xCal, yCal, zCal;
	int i;

	xCal = 0;
	yCal = 0;
	zCal = 0;
	for(i = 0; i < samples; i++) {
		int x, y, z;

		if(neo_accel_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		xCal += (float)(x);
		yCal += (float)(y);
		zCal += (float)(z);

		usleep(1000 * delayEach);
	}

	neo_accel_calibration[0] = (int)(xCal / ((float)samples));
	neo_accel_calibration[1] = (int)(yCal / ((float)samples));
	neo_accel_calibration[2] = (int)(zCal / ((float)samples));

	return NEO_OK;
}

int neo_accel_free() {
	if(neo_accel_freed == 0) {
		FILE* enabler;
		enabler = fopen(ACCELENABLE, "w");
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 1);
		fflush(enabler);
		fclose(enabler);
		neo_accel_freed = 2;
	}

	return NEO_OK;
}


/*int main() {
	int x, y, z;
	neo_accel_init();
	neo_accel_calibrate(20, 100);
	while(1) {
		neo_accel_read_calibrated(&x, &y, &z);	
		printf("X: %d Y: %d Z: %d\n", x, y, z);
		usleep(1000*20);
	}
	neo_accel_free();
}*/
