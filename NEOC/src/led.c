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
 * @file led.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief File to control the Builtin status LED on the board
 *
 * @details This is the source file that easily allows anyone to attach their program to
 * the BuiltinLED that by default is not connected to a standard pin. This is the
 * RED LED that indicates the file disk IO. This will be auto released on program 
 * exit, so no reason to call neo_led_free unless you want to
 */
#include <neo.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef DOXYGEN_SKIP

//The main brightness controller in /sys
FILE *neo_led_controller;

//Double free or init check to make sure it's not initialized or freed multiple times
unsigned char neo_led_freed = 2;

#endif

/**
 * @brief Initializes the Builtin LED on the Udoo
 * 
 * This will attach the Builtin LED to the current program
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 *
 * @warning When using this you must be ROOT not sudo but the user root
 * @note To run as root try sudo su and then run it or sudo su -c './a.out'
 */
int neo_led_init() {
	//Check for root permissions
	neo_check_root("The builtin LED requires root permission!");
	//Make sure it's not already initialized
	if(neo_led_freed == 2) {
		//Open the sys path to write the LED brightness to
		neo_led_controller = fopen(LEDPATH, "w");
		if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;
		neo_led_freed = 0; //Set the global initialized flag
	}
	return NEO_OK;
}

/**
 * @brief Sets the Builtin LED state
 * 
 * This will remove the LED from the program space and return it to the system
 *
 * @param state The LED state to set it to either LOW (0) aka off or HIGH (1) aka on 
 *
 * @return NEO_OK/NEO_UNUSABLE or NEO_LED_STATE_ERROR if something went wrong
 *
 * @note Anything besides a 0 or 1 for the led state will return NEO_LED_STATE_ERROR
 */
int neo_led_set(int state) {
	if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;
	if(state < 0 || state > 1) return NEO_LED_STATE_ERROR;

	fflush(neo_led_controller); //Clear the buffer
	fseek(neo_led_controller, 0, SEEK_SET); //Set the seek position to 0
	fprintf(neo_led_controller, "%d", state); //Write the new brightness state
	fflush(neo_led_controller); //Finish with the controller by flushing the buffer
	return NEO_OK;
}

/**
 * @brief Attempts to turn the LED on
 * 
 * This calls the neo_led_set in the backend with the parameter 1
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 */
int neo_led_on() {
	return neo_led_set(1); //Turn the LED on
}

/**
 * @brief Attempts to turn the LED off
 * 
 * This calls the neo_led_set in the backend with the parameter 0
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 */
int neo_led_off() {
	return neo_led_set(0); //Turn the LED off
}

/**
 * @brief De-ionitializes the Builtin LED on the Udoo
 * 
 * This will remove the LED from the program space and return it to the system
 * 
 * @return NEO_OK or NEO_UNUSABLE error if something went wrong
 */
int neo_led_free() {
	if(neo_led_freed == 0) {
		if(neo_led_controller == NULL) return NEO_UNUSABLE_ERROR;	
		fclose(neo_led_controller);
		neo_led_freed = 2;
	}
	return NEO_OK;
}
