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
 * @file accel.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief File to control the Builtin Accelerometer
 *
 * @details This source file holds the functions to easily allow anyone to have easy access
 * to the accelerometer and calibration data. This will be auto released on program 
 * exit, so no reason to call neo_led_free unless you want to
 */
#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef DOXYGEN_SKIP

//The accelerometer data sysfs file
FILE *neo_accel_data;

//The values to adjust the accelerometer reading data
int neo_accel_calibration[] = {0, 0, 0};

//The double free or init flag to make sure it doesn't get called twice
unsigned char neo_accel_freed = 2;

#endif

/**
 * @brief Sets the update rate of the accelerometer
 * 
 * Usually you will probably run your code within some type of loop that
 * runs on forever. Well obviously that you are probably using the accelerometer
 * you are using a loop. Check the delay on that loop and set it to the rate below!
 * If you make the pull too fast the accelerometer will reset its values, if you pull too slow
 * then the values you pull might be the same when you pull again. Try making the value
 * slightly higher than what you pull, it will save you some processing
 *
 * @param rate The milliseconds to pull the new value, every loop
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note This would be way better if ran as root rather than sudo
 */
int neo_accel_set_poll(int rate) {
	FILE *polling;
	
	//Open the temporary poll rate
	polling = fopen(ACCELPOLLP, "w");

	//Safety checkt to see if the pull rate is not there
	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	//Double check the rate, the seek should default be 0
	fprintf(polling, "%d", rate);
	fflush(polling); //Double flush the buffer
	fclose(polling); //Close it

	return NEO_OK;
}


/**
 * @brief Initializes the Builtin Accelerometer
 * 
 * Attacht the Builtin Accelerometer to the program for reading. The setPoll
 * may be called anytime AFTER the init method. Just remember to set it
 * since it's consistent with the kernel on reboot!
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @warning Please use ROOT when running this and not just sudo
 * @note To run as root try sudo su and then run it or sudo su -c './a.out'
 */
int neo_accel_init() {
	//Double check if it has already been init
	if(neo_accel_freed == 2) {
		neo_check_root("Accelerometer requires root access!"); //Double check root access
	
		//Enable the 
		FILE *enabler;
		enabler = fopen(ACCELENABLE, "w"); //Attempt to open the enabler
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR; //Make sure we have access to the accel
	
		fprintf(enabler, "%d", 1); //Enable accel
		fflush(enabler); //Flush the buffer
		fclose(enabler); //Close the enabler
	
		neo_accel_data = fopen(neo_accel_data, "r");
		
		if(neo_accel_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_accel_freed = 0; //Set the 
		
		return neo_accel_set_poll(ACCELPOLL); //Set the accel_poll
	}

	return NEO_OK;
}

/**
 * @brief Reads the raw data from the accelerometer
 * 
 * This will read the raw CURRENTLY updated data from the accelerometer
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the accel values reset to 0, making the differencials
 * a lot smaller.
 *
 * @param x A pointer to the X value of the accelerometer
 * @param Y A pointer to the Y value of the accelerometer
 * @param Z A pointer to the Z value of the accelerometer
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you accel_read
 */
int neo_accel_read(int *x, int *y, int *z) {
	//Double check to see if the accel is still usable
	if(neo_accel_data == NULL) return NEO_UNUSABLE_ERROR;

	fflush(neo_accel_data); //Flush the buffer to get an update value like a sync
	fseek(neo_accel_data, 0, SEEK_SET); //Reset the seek back to zero
	if(fscanf(neo_accel_data, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR; //Read from the accel data
	return NEO_OK;
}

/**
 * @brief Reads the calibrated data from the accelerometer
 * 
 * This will read the raw CURRENTLY updated and calibrated data from the accelerometer
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the accel values reset to 0, making the differencials
 * a lot smaller. The calibration method must be called to actually do anything, if it's not called
 * there is no point in calling this method.
 *
 * @param x A pointer to the X value of the accelerometer (calibrated)
 * @param Y A pointer to the Y value of the accelerometer (calibrated)
 * @param Z A pointer to the Z value of the accelerometer (calibrated)
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you accel_read_calibrated
 */
int neo_accel_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_accel_read(x, y, z); //Read the raw data

	//Fix the calibrated values from the beginning to average out the values
	(*x) += (neo_accel_calibration[0] > 0) ? -(neo_accel_calibration[0]) : neo_accel_calibration[0];
	(*y) -= (neo_accel_calibration[1] > 0) ? -(neo_accel_calibration[1]) : neo_accel_calibration[1];
	(*z) -= (neo_accel_calibration[2] > 0) ? -(neo_accel_calibration[2]) : neo_accel_calibration[2];

	return okRet; //Check the return codes
}

/**
 * @brief Calibrate the accelerometer over a period of time
 * 
 * This will calibrate the accelerometer and try to set all the axis to 0. Make sure
 * The Udoo is not moving during this time to get a proper zeroed calibration.
 *
 * @param samples The amount of samples to take
 * @param delayEach The amount of millisecond delays between each sample
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note The total time can be calculated via samples * delayEach equals total millis
 */
int neo_accel_calibrate(int samples, int delayEach) {
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
		if(neo_accel_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		//Add each axis to the average
		xCal += (float)(x);
		yCal += (float)(y);
		zCal += (float)(z);

		//Wait for each of the delays
		usleep(1000 * delayEach);
	}

	//Set the accelerometer values based on the averaged samples
	neo_accel_calibration[0] = (int)(xCal / ((float)samples)); //X
	neo_accel_calibration[1] = (int)(yCal / ((float)samples)); //Y
	neo_accel_calibration[2] = (int)(zCal / ((float)samples)); //Z

	return NEO_OK;
}

int neo_accel_free() {
	//Double check that the free is released
	if(neo_accel_freed == 0) {
		FILE* enabler;
		enabler = fopen(ACCELENABLE, "w"); //Open the enabler
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 0); //Disable the accelerometer on release
		fflush(enabler); //Flush the value for safety
		fclose(enabler); //Close it
		
		fclose(neo_accel_data); //Close the data reader
		neo_accel_freed = 2; //Set global falg to set the free to already freed
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
