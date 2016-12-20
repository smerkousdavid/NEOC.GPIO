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
 * Reading. If you your main loop is 50 millis then set the poll to something
 * around 50 as well.
 * 
 */

#include <neo.h>

#ifndef DOXYGEN_SKIP

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

#endif

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
 *
 * @note This would be way better if ran as root rather than sudo
 */
int neo_magno_set_poll(int rate) {
	FILE *polling;
	
	//Open the temporary poll rate
	polling = fopen(MAGNOPOLLP, "w");

	//Safety checkt to see if the pull rate is not there	
	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	//Double check the rate, the seek should already be 0
	fprintf(polling, "%d", rate);
	fflush(polling); //Double flush the buffer
	fclose(polling); //Close it

	return NEO_OK;
}

/**
 * @brief Initializes the Builtin Magno
 * 
 * Attach the Builtin Magno to the program for reading. The setPoll
 * may be called anytime AFTER the init method. Just remember to set it
 * since it's consistent with the kernel on reboot!
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @warning Please use ROOT when running this and not just sudo
 * @note To run as root try sudo su and then run it or sudo su -c './a.out'
 */
int neo_magno_init() {
	//Double check if it has already been init
	if(neo_magno_freed == 2) {
		//Setup cleanup on exit of application
		if(neo_exit_set == 2) {
			atexit(neo_free_all);
			neo_exit_set = 1;
		}
		
		//Double check root access
		neo_check_root("Magno requires root access!");
	
		//Enable the magno
		FILE *enabler;
		enabler = fopen(MAGNOENABLE, "w"); //Attempt to open the enabler
		if(enabler == NULL) return NEO_UNUSABLE_ERROR; //Make sure we have access to the magno
		
		fprintf(enabler, "%d", 1); //Enable magno
		fflush(enabler); //Flush the buffer
		fclose(enabler); //Close the enabler
	
		neo_magno_data = fopen(MAGNODATA, "r");
		
		if(neo_magno_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_magno_freed = 0; //Set the global init flag
		
		return neo_magno_set_poll(MAGNOPOLL);
	}
	
	return NEO_OK;
}

/**
 * @brief Reads the raw data from the magno
 * 
 * This will read the raw CURRENTLY updated data from the magno
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the magno values reset to 0, making 
 * the differencials a lot smaller.
 *
 * @param x A pointer to the X value of the magno
 * @param Y A pointer to the Y value of the magno
 * @param Z A pointer to the Z value of the magno
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you magno_read
 */
int neo_magno_read(int *x, int *y, int *z) {
	//Double check to see if the magno is still usable
	if(neo_magno_data == NULL) return NEO_UNUSABLE_ERROR;
	
	fflush(neo_magno_data); //Flish the buffer to get an update value like a sync
	fseek(neo_magno_data, 0, SEEK_SET); //Reset the seek back to zero
	if(fscanf(neo_magno_data, "%d,%d,%d", x, y, z) == EOF) 
		return NEO_READ_ERROR; //Read data from the magno data file
	return NEO_OK;
}

/**
 * @brief Reads the calibrated data from the magno
 * 
 * This will read the raw CURRENTLY updated and magno data from the magno
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the magno values reset to 0, making the
 * differencials a lot smaller. The calibration method must be called to
 * actually do anything, if it's not called there is no point in calling 
 * this method.
 *
 * @param X A pointer to the X value of the magno (calibrated)
 * @param Y A pointer to the Y value of the magno (calibrated)
 * @param Z A pointer to the Z value of the magno (calibrated)
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you magno_read_calibrated
 */
int neo_magno_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_magno_read(x, y, z); //Read the raw data

	//Fix the calibrated values from the beginning to average out the values
	(*x) += (neo_magno_calibration[0] > 0) ? -(neo_magno_calibration[0]) : neo_magno_calibration[0]; 
	(*y) -= (neo_magno_calibration[1] > 0) ? -(neo_magno_calibration[1]) : neo_magno_calibration[1];
	(*z) -= (neo_magno_calibration[2] > 0) ? -(neo_magno_calibration[2]) : neo_magno_calibration[2];

	return okRet; //Check the return codes
}

/**
 * @brief Calibrate the magno over a period of time
 * 
 * This will calibrate the magno and try to set all the axis to 0. Make sure
 * The Udoo is not moving during this time to get a proper zeroed calibration.
 *
 * @param samples The amount of samples to take
 * @param delayEach The amount of millisecond delays between each sample
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note The total time can be calculated via samples * delayEach equals total millis
 */
int neo_magno_calibrate(int samples, int delayEach) {
	float xCal, yCal, zCal; //To set the average of the neo_accel_calibrated
	int i;

	//Global averages will be calculated by adding the temporary axis
	xCal = 0;
	yCal = 0;
	zCal = 0;
	
	//Loop for x samples with a delay each sample simple math
	for(i = 0; i < samples; i++) {
		int x, y, z; //Temporary rest of axis

		//Make sure we can read before adding to the average
		if(neo_magno_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		//Add each axis to the average
		xCal += ((float)x);
		yCal += ((float)y);
		zCal += ((float)z);

		//Wait for each of the delays
		usleep(1000 * delayEach);
	}

	//Set the accelerometer values based on the averaged samples
	neo_magno_calibration[0] = (int)(xCal / ((float)samples)); //X
	neo_magno_calibration[1] = (int)(yCal / ((float)samples)); //Y
	neo_magno_calibration[2] = (int)(zCal / ((float)samples)); //Z

	return NEO_OK;
}

/**
 * @brief Frees the builtin Magno
 * 
 * Remove the Magno reading from the program and disable the driver on the
 * system this will save processing power and use less power. This will be
 * called on a CLEAN system exit but when forced like Ctrl-C, 
 * it won't properly release and will still run the driver on the system.
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 */
int neo_magno_free() {
	//Double check that the free is released
	if(neo_magno_freed == 0) {
		FILE* enabler;
		enabler = fopen(MAGNOENABLE, "w"); //Open the enabler
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 0); //Disable the magno on release
		fflush(enabler); //Flush the value for safety
		fclose(enabler); //Close it
		
		fclose(neo_magno_data); //Close the data reader
		neo_magno_freed = 2; //Set global flag to set the free to already freed 
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

