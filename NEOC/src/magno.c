#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

FILE *neo_magno_data;

int neo_magno_calibration[] = {0, 0, 0};

unsigned char neo_magno_freed = 2;

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
	if(neo_magno_freed == 2) {
		neo_check_root("Magno requires root access!");
	
		FILE *enabler;
		enabler = fopen(MAGNOENABLE, "w");
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
		
		fprintf(enabler, "%d", 1); //Enable accel
		fflush(enabler);
		fclose(enabler);
	
		neo_magno_data = fopen(neo_magno_data, "r");
		
		if(neo_magno_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_magno_freed = 0;	
	}
	return neo_magno_set_poll(MAGNOPOLL);
}


int neo_magno_read(int *x, int *y, int *z) {
	if(neo_magno_data == NULL) return NEO_UNUSABLE_ERROR;

	fflush(neo_magno_data);
	fseek(neo_magno_data, 0, SEEK_SET);
	if(fscanf(neo_magno_data, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR;
	return NEO_OK;
}

int neo_magno_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_magno_read(x, y, z);

	(*x) += (neo_magno_calibration[0] > 0) ? -(neo_magno_calibration[0]) : neo_magno_calibration[0];
	(*y) -= (neo_magno_calibration[1] > 0) ? -(neo_magno_calibration[1]) : neo_magno_calibration[1];
	(*z) -= (neo_magno_calibration[2] > 0) ? -(neo_magno_calibration[2]) : neo_magno_calibration[2];

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

	neo_magno_calibration[0] = (int)(xCal / ((float)samples));
	neo_magno_calibration[1] = (int)(yCal / ((float)samples));
	neo_magno_calibration[2] = (int)(zCal / ((float)samples));

	return NEO_OK;
}

int neo_magno_free() {
	if(neo_magno_freed == 2) {
		FILE* enabler;
		enabler = fopen(MAGNOENABLE, "w");
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 1);
		fflush(enabler);
		fclose(enabler);
		neo_magno_freed = 0;
	}

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

