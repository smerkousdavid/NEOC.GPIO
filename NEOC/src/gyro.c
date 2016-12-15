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
 * @file gyro.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief File to control the Builtin Gyro
 *
 * @details This source file holds the functions to easily allow anyone to have easy access
 * to the gyro and calibration data. This will be auto released on program 
 * exit, so no reason to call neo_gyro_free unless you want to
 */
#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef DOXYGEN_SKIP

//The gyro data sysfs file
FILE *neo_gyro_data;

//The values to adjust the gyro data
int neo_gyro_calibration[] = {0, 0, 0};

//The double free and init flag so it doesn't get called twice
unsigned char neo_gyro_freed = 2;

#endif

/**
 * @brief Sets the update rate of the gyro
 * 
 * Usually you will probably run your code within some type of loop that
 * runs on forever. Well obviously that you are probably using the gyro
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
int neo_gyro_set_poll(int rate) {
	FILE *polling;
	
	//Open the temporary poll rate
	polling = fopen(GYROPOLLP, "w");

	//Safety check to make sure the pull rate is there
	if(polling == NULL) return NEO_UNUSABLE_ERROR;

	fprintf(polling, "%d", rate); //Push new rate
	fflush(polling); //Flush buffer
	fclose(polling); //Close buffer
	return NEO_OK;
}

/**
 * @brief Initializes the Builtin Gyro
 * 
 * Attach the Builtin Gyro to the program for reading. The setPoll
 * may be called anytime AFTER the init method. Just remember to set it
 * since it's consistent with the kernel on reboot!
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @warning Please use ROOT when running this and not just sudo
 * @note To run as root try sudo su and then run it or sudo su -c './a.out'
 */
int neo_gyro_init() {
	//Double check if it has already been init
	if(neo_gyro_freed == 2) {
		//Setup cleanup on exit of application
		if(neo_exit_set == 2) {
			atexit(neo_free_all);
			neo_exit_set = 1;
		}
		neo_check_root("Gyro requires root access!"); //Double check root access
	
		FILE *enabler;
		enabler = fopen(GYROENABLE, "w"); //Open up the enabler
	
		if(enabler == NULL) return NEO_UNUSABLE_ERROR; //Gyro safety check
		
		fprintf(enabler, "%d", 1); //Enable accel
		fflush(enabler); //Flush the new buffer
		fclose(enabler); //Close the stream
	
		neo_gyro_data = fopen(GYRODATA, "r");
		
		if(neo_gyro_data == NULL) return NEO_UNUSABLE_ERROR;
		neo_gyro_freed = 0; //Set the global init flag
		
		return neo_gyro_set_poll(GYROPOLL); //Set the default poll rate
	}
	return NEO_OK;
}

/**
 * @brief Reads the raw data from the gyro
 * 
 * This will read the raw CURRENTLY updated data from the gyro
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the gyro values reset to 0, making the differencials
 * a lot smaller.
 *
 * @param X A pointer to the X value of the gyro
 * @param Y A pointer to the Y value of the gyro
 * @param Z A pointer to the Z value of the gyro
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you gyro_read
 */
int neo_gyro_read(int *x, int *y, int *z) {
	//Double check to see if the accel is still usable
	if(neo_gyro_data == NULL) return NEO_UNUSABLE_ERROR;

	fflush(neo_gyro_data); //Flush and sync the read
	fseek(neo_gyro_data, 0, SEEK_SET); //Reset the seek back to zero
	if(fscanf(neo_gyro_data, "%d,%d,%d", x, y, z) == EOF) return NEO_READ_ERROR; //Update the pointers with the new values
	return NEO_OK;
}

/**
 * @brief Reads the calibrated data from the gyro
 * 
 * This will read the raw CURRENTLY updated and calibrated data from the gyro
 * make sure you update the pollRate to get the most updated values. Be careful
 * the faster the poll rate the faster the gyro values reset to 0, making the differencials
 * a lot smaller. The calibration method must be called to actually do anything, if it's not called
 * there is no point in calling this method.
 *
 * @param x A pointer to the X value of the gyro (calibrated)
 * @param Y A pointer to the Y value of the gyro (calibrated)
 * @param Z A pointer to the Z value of the gyro (calibrated)
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @note Set the poll rate to the same amount of delay you gyro_read_calibrated
 */
int neo_gyro_read_calibrated(int *x, int *y, int *z) {
	int okRet = neo_gyro_read(x, y, z); //Update and read the raw data

	//Fix the calibrated values from the beginning to average out the values
	(*x) += (neo_gyro_calibration[0] > 0) ? -(neo_gyro_calibration[0]) : neo_gyro_calibration[0];
	(*y) -= (neo_gyro_calibration[1] > 0) ? -(neo_gyro_calibration[1]) : neo_gyro_calibration[1];
	(*z) -= (neo_gyro_calibration[2] > 0) ? -(neo_gyro_calibration[2]) : neo_gyro_calibration[2];

	return okRet; //Return the read results since calibration is just a simple subtraction or addition
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
int neo_gyro_calibrate(int samples, int delayEach) {
	float xCal, yCal, zCal; //To set the average of the neo_gyro_calibrated
	int i;

	//Global averages will be calculated by adding the temporary axis
	xCal = 0;
	yCal = 0;
	zCal = 0;

	//Loop for x samples with a delay each sample simple math
	for(i = 0; i < samples; i++) {
		int x, y, z; //Temporary rest of axis

		//Double check safety of calibrated read before actually changing the data
		if(neo_gyro_read(&x, &y, &z) == NEO_UNUSABLE_ERROR) return
				NEO_UNUSABLE_ERROR;

		//Add each axis to the average
		xCal += ((float)x);
		yCal += ((float)y);
		zCal += ((float)z);

		//Wait for each of the samples
		usleep(1000 * delayEach);
	}

	//Update the global calibrated averages
	neo_gyro_calibration[0] = (int)(xCal / ((float)samples)); //X
	neo_gyro_calibration[1] = (int)(yCal / ((float)samples)); //Y
	neo_gyro_calibration[2] = (int)(zCal / ((float)samples)); //Z

	return NEO_OK;
}

/**
 * @brief Frees the builtin Gyro
 * 
 * Remove the Gyro reading from the program and disable the driver on the system
 * this will save processing power and use less power. This will be called on a CLEAN
 * system exit but when forced like Ctrl-C, it won't properly release and will still run
 * the driver on the system
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 */
int neo_gyro_free() {
	//Double check the free is released
	if(neo_gyro_freed == 0) {
		FILE* enabler;
		enabler = fopen(GYROENABLE, "w"); //Open the enabler
	
		//Safety check to see if the enabler is valid
		if(enabler == NULL) return NEO_UNUSABLE_ERROR;
	
		fprintf(enabler, "%d", 0); //Update the disabled flag
		fflush(enabler); //Flush the buffer
		fclose(enabler); //Close it
		neo_gyro_freed = 2; //Set global flag to freed
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
