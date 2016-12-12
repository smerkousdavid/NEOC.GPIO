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
 * @file neo.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief Core C file that includes base functionality used by most files
 *
 * NEOC was designed as an extremely fast way of managing the iio and pinout
 * On all the Udoo NEO boards. This is supported only for NEO but all subversions
 * This library was mainly written in C with C++ mix for the reason of fake PWM.
 * Check out the repo. And other repos for bindings for this main code
 * 
 * @see https://github.com/smerkousdavid/NEOC.GPIO
 * @see https://github.com/smerkousdavid/Neo.GPIO
 * 
 */
 
#include <neo.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * @brief Linear remapping based on Arduino
 *
 * Lets just say analog is 0 - 1024 and pwm is 0 - 100 (Not actually)
 * Then I could just do 50pwm to get analog output of 512
 * @param value The input value you want to scale
 * @param omin The input minimum value
 * @param omax The input maximum value
 * @param nmin The output minimum value
 * @param nmax The output maximum value
 * 
 * @return The scaled value of the first argyument @p value
 * 
 * @note there's only a float version for C
 */
float neo_re_map(float value, float omin, float omax, float nmin, float nmax)
{
    return ((value - omin) / (omax - omin)) * (nmax - nmin) + nmin;
}

/**
 * @brief Root checking
 *
 * Checks if root with custom response
 * @param saying the response on fail
 */
void neo_check_root(char const * saying) {
	if(geteuid() != 0) { //root is always zero (effective user id)
		fprintf(stderr, "No root privlegdes on UID(%d)! %s\n", geteuid(), saying);
		exit(1); //No need to proceed
		//@TODO - Implement a setuid() or password entry
	}
}

/**
 * @brief Root checking with only return
 *
 * Checks if root with return of checking
 *
 * @return either NEO_OK if root or NEO_FAIL when other
 *
 * @note Please run the scripts as su not sudo to have full permissions
 */
int neo_check_root_return() {
	if(geteuid() != 0) { //root is always zero (effective user id)
		return NEO_FAIL;
	}
	return NEO_OK;
}

/**
 * @brief m4 core disabling
 *
 * When ran this disables the arduino side of the NEO
 * @return either NEO_OK or NEO_FAIL
 */
int neo_disable_m4() {
	neo_check_root("Can't disable m4 without root!");
	return (system("udoom4ctl disable") == 0) ? NEO_OK : NEO_FAIL;
}

/**
 * @brief m4 core enabling
 *
 * When ran this enables the arduino side of the NEO
 * @return either NEO_OK or NEO_FAIL
 */
int neo_enable_m4() {
	neo_check_root("Can't enable m4 without root!");
	return (system("udoom4ctl enable") == 0) ? NEO_OK : NEO_FAIL;
}

/**
 * @brief core screen change type 
 * 
 * When ran this changes the output of your screen to the lvds 7 inch screen
 * @return either NEO_OK or NEO_FAIL
 */

int neo_screen_set_lvds() {
	neo_check_root("To set to lvds you must be root!");
	return (system("udooscreenctl set lvds7") == 0) ? NEO_OK : NEO_FAIL;
}

/**
 * @brief core hdmi screen change type
 * 
 * When ran this changes the output of your screen to the hdmi port
 * @return either NEO_OK or NEO_FAIL
 */

int neo_screen_set_hdmi() {
	neo_check_root("To set to hdmi you must be root!");
	return (system("udooscreenctl set hdmi") == 0) ? NEO_OK : NEO_FAIL;
}
