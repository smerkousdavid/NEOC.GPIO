/*----------------------------------------------------------------------||
|                                                                        |
| Copyright (C) 2016 by David Smerkous                                   |
| License Date: 11/27/2016                                               |
| Modifiers: none                                                        |
|                                                                        |
| NEOC (libneo) is free software: you can redistribute it and/or modify  |
|   it under the terms of the GNU General Public License as published by |
|   the Free Software Foundation, either version 3 of the License, or    |
|   (at your option) any later version.                                  |
|                                                                        |
| NEOC (libneo) is distributed in the hope that it will be useful,       |
|   but WITHOUT ANY WARRANTY; without even the implied warranty of       |
|   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        |
|   GNU General Public License for more details.                         |
|                                                                        |
| You should have received a copy of the GNU General Public License      |
|   along with this program.  If not, see http://www.gnu.org/licenses/   |
|                                                                        |
||----------------------------------------------------------------------*/

/**
 * 
 * @file magno.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief Magno file that contains all mapping and control over the magno
 *
 * This file allows to read the raw and calibrated data from the magno
 * It's recommended to set the poll rate of the update to get the most accurate
 * Reading. If you your main loop is 50 millis then set the poll to something around
 * 50 as well.
 * 
 */

#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//Magno read data file pointer
FILE *neo_magno_data;


//The add sub vals for the calibrator
int neo_magno_calibration[] = {0, 0, 0};

//Double free or error fixer also no need to double initialize
unsigned char neo_magno_freed = 2;

/**
 * @brief Sets millis pulling rate of magno
 *
 * This will pull the new magnometer values every X millis
 * The faster the poll rate the more CPU will be used to process the values
 * Also the more distant the values will be. It's recommended to only up this value
 * If you don't set the poll rate fast enough you might get the same value twice when
 * using the read method
 * 
 * @param rate The value in millis to pull the new magno values at
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERROR if magno is not found
 * 
 * @note NEO_UNUSABLE_ERROR means that there isn't a magno detected
 */
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

