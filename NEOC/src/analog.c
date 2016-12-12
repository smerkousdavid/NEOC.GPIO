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
 * @file analog.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief The main file to control the Analog A0 through A5 pins
 *
 * This source file is the core of all Analog control. This doesn't go through
 * the m4 or read via serial. This is directly accessing the pins via iio on the
 * A9 side. So you can expect fast results. Root is usually required. I have had it where
 * it ran fine, iio can be kind of quirky. You can either read via .read() or readRaw() to
 * get the fully mapped 0 to 4095 12 bit value.
 * 
 * @note Please disable the m4 core before continuing to use neo_analog_init()
 */
#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef DOXYGEN_SKIP

//Available analog ports to proper index mapping ex: (iio bank 0 and pin 0)
const char * const ANALOGPORTS[][2] = {{"0", "0"}, {"0", "1"}, {"0", "2"}, {"0", "3"}, {"1", "0"}, {"1", "1"}}; 

//2 safety for each array
//These hold the scaled value if they want to manually read the scale and the initializer
unsigned char USABLEANALOG[GPIOPORTSL + 2];
float  ANALOGSCALE[ANALOGSCALEL + 2];

//sysfs iio combination file holders
FILE* analogR[GPIOPORTSL + 2];

//Double free or initialize check, make sure that it doesn't fail on release
unsigned char neo_analog_freed = 2;

#endif

/**
 * @brief Initializes the Analog iio
 * 
 * This opens up all of the Analog pins up for reading. The reason why all of them
 * are open is because it's faster for one and for two there is no reason for the m4
 * to be using them, so let our program use it all.
 * 
 * @return NEO_OK/NEO_UNUSABLE_ERROR/NEO_EXPORT_ERROR if some Analog weren't initialized
 * 
 * @note Do not provide over 3.3v to the Analog pins or you will ruin the board! 
 */
int neo_analog_init() {
	int i, gi, fail;

	fail = NEO_OK; //Return code
	
	//Double check to see if it's already initialized
	if(neo_analog_freed == 2) {
		//Set all the pins as usable
		for(gi = 0; gi <= ANALOGPORTSL; gi++) {USABLEANALOG[gi] = 1;}

	//If we are scaling the analog set an initial safety scale
	#ifdef SCALEANALOG
		for(gi = 0; gi <= ANALOGSCALEL; gi++) {ANALOGSCALE[gi] = 0.25;}
	#endif

	#ifdef SCALEANALOG	
		//For each iio bank find the analog scaling value
		for(i = 0; i <= ANALOGSCALEL; i++) { 
			size_t scaleSize = 10 + analogL + analogBL + analogSL; //Safety for buffer size
			char buff[scaleSize]; //Store new path in buffer
			//Compile the scaling path
			sprintf(buff, "%s%d%s%s", ANALOGPATHP, i, ANALOGBASEP, ANALOGSCALEP);

			FILE *sFile;
			sFile = fopen(buff, "r"); //Get the scale from iio/sysfs

			if(sFile == NULL) {
				fail = NEO_EXPORT_ERROR;
				continue;
			}

			//Set float to store scale to
			float curScale;
			curScale  = 0.25f;
			fscanf(sFile, "%f", &curScale); //Load the scale float
			fclose(sFile);
			ANALOGSCALE[i] = curScale; //Store that scale for temporary iio bank
		}
	#endif

		//Open up all of the analog ports
		for(i = 0; i <= ANALOGPORTSL; i++) {
			size_t rawSize = 3 + analogL + analogBL + analogRL; //Add safety to the opening path

			char buffR[rawSize]; //Create the new path buffer
				
			sprintf(buffR, "%s%s%s%s%s", ANALOGPATHP, ANALOGPORTS[i][0],
						ANALOGBASEP, ANALOGPORTS[i][1], ANALOGRAWP); //Save the analog ports path into that requested buffer
				
			analogR[i] = fopen(buffR, "r"); //Open the Analog reading port
			//Double check to see that the Analog pin is usable
			if(analogR[i] == NULL) {
				fail = NEO_UNUSABLE_ERROR; //Set the unusable flag and continue
				USABLEANALOG[i] = 0;
			}
		}
		neo_analog_freed = 0; //Set global initialized flag
	}
	return fail;
}

/**
 * @brief Main read raw method for analog
 * 
 * This will directly read from the analog pin and give you the raw returned
 * 12 bit value
 * 
 * @param pin The analog pin (0 to 5) to read from
 * 
 * @return a float of (0 -> 4095)/NEO_UNUSABLE_ERROR/NEO_PIN_ERROR/NEO_READ_ERROR if it failed to read the pin
 * 
 * @note If you have recieved the NEO_UNUSABLE_ERROR, that probably means you aren't root
 */
float neo_analog_read(int pin) {
	//Safety check the pin
	if(pin < 0 || pin > ANALOGPORTSL) return NEO_PIN_ERROR;

	FILE *curR = analogR[pin];

	//Double check that the pin is usable
	if(curR == NULL || !USABLEANALOG[pin]) return NEO_UNUSABLE_ERROR;

	float curRaw;
	
	fflush(curR); //Update buffer before reading (necessary)
	fseek(curR, 0, SEEK_SET);

	if(fscanf(curR, "%f", &curRaw) == EOF) return NEO_READ_ERROR; //If at end of file (shouldn't happen) then failed to read

#ifdef SCALEANALOG
	int curScale;
	//Double check the scaling value (it should never change);
	if(sscanf(ANALOGPORTS[pin][0], "%d", &curScale) == EOF) return NEO_READ_ERROR;
	
	if(curScale < 0 || curScale > ANALOGSCALEL) return NEO_SCALE_ERROR;
	//Used fixed scaling if enabled
	return curRaw * ANALOGSCALE[curScale];
#endif
	return curRaw; //If no scaling specified then return the raw value
}

/**
 * @brief Analog freeing method
 * 
 * This will release the analog pins from the program and free them up to be
 * used with another. The init and free methods should be extremely fast and is fine to
 * call multiple times.
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERROR if it failed to release a pin
 */
int neo_analog_free() 
{
	int fail;
	int i;

	fail = NEO_OK;

	if(neo_analog_freed == 0) {
		for(i = 0; i < ANALOGPORTSL; i++) {
			if(USABLEANALOG[i]) {
				FILE *curR = analogR[i];

				if(curR != NULL) fclose(curR);
				else fail = NEO_UNUSABLE_ERROR;
			}
		}
		neo_analog_freed = 2;
	}
	return fail;
}

/** \page exampleanalog Read
 * \breif How to read the voltage and raw value from Analog
 *
 * This tutorial is just a quick brief overview of how to read Analog
 * both the voltage and the raw 12bit value. There is an optional #define you can
 * place if you want that raw value to be scaled to millivolts.<BR><BR>
 *
 * By now you should know how to create, read and write to a file. So lets create one
 * called analogread.c or analogread.cpp (for c++)
 * 
 * \section examplec C Example
 *
 *  \code{.c}
 *  #include <neo.h>
 *  #include <time.h>
 *
 * //OPTIONAL SCALED VALUE
 * //#define SCALEANALOG
 *
 * int main() {
 *     neo_disable_m4(); //Make sure you disblae the m4 core (this doesn't need to be ran everytime)
 *
 *     int ret; //Variable to hold the return
 *     ret = neo_analog_init();
 *
 *     if(ret != NEO_OK) {
 *        printf("FAILED INITIAL SETUP FOR ANALOG!\n");
 *        return 1;
 *     }
 *
 *     int i, id;
 *
 *     for(i = 0; i < 3; i++) {
 *         printf("Trial %d\n", i);
 *         for(id = 0; id <= 5; id++) {
 *             printf("ANALOG(A%d): %.2f out of 4095\n", id, neo_analog_read(id));
 *         }
 *         printf("\n\n");
 *         sleep(1);
 *     }
 *     neo_analog_free();
 *     printf("Done reading analog!\n");
 *     return 0;
 * }
 *  \endcode
 * <BR>
 * \section examplecpp C++ Example
 * \code{.cpp}
 *  #include <neo.h>
 *  #include <iostream>
 * 
 * //OPTIONAL SCALED VALUE (If the value is over 3.3v use this, either than that don't)
 * //#define SCALEANALOG
 *
 *  //using namespace neo; //Usually this is bad practice 
 *  using namespace std;
 *
 *  //Main function
 *  int main() {
 *      cout << "Disabling m4 core" << endl;
 *		neo::setM4(neo::DISABLED);
 *      
 *      neo::Analog zeroPin(0); //Read from A0
 *      float rawValue = zeroPin.readRaw(); //Reads raw
 *      float voltage = zeroPin.read(); //Voltage auto calculated      
 *
 *      cout << "A0 voltage: " << voltage << " RAW: " << rawValue << endl;
 *
 *      return 0;
 *  }
 * \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc analogread.c -I/usr/include -lneo -o analogread
 *   chmod 755 analogread
 *   ./analogread
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ analogread.cpp -I/usr/include -lneo -o analogread
 * \endcode
 * <BR>
 * \section results Results
 * Try hooking up an LED to that PWM pin and setting the PWM values at different points to see if the lighting changes
 * If nothing changes make sure that pin supports REAL PWM, isn't failing to initialize it and it's been pinmuxed properly
 * In the device tree editor
 */

/*int main() {
	printf("INIT %d\n", neo_analog_init());
	while(1) {
		printf("MODE %f\n", neo_analog_read(0));
		sleep(1);	
	}
	printf("FREE %d\n", neo_analog_free());
	return 0;
}*/
