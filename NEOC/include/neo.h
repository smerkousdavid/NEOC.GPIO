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
 * @file neo.h
 * @author David Smerkous
 * @date 11/27/2016
 * @brief Header file (C/C++) that includes all functions for the libneo.so
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

//Doxygen mainpage generation

/** \mainpage NEOC Official Documentation
 * 
 *  \section intro_sec Introduction
 *
 * NEOC is a C and C++ library that allows low level and extremely fast control over the Udoo
 * Neo. NEOC allows control over all internal and brick aspects of the Udoo. This library is easy to install
 * Since it's already precompiled and requires only one header for all the functions. Note, the C++ classes are 
 * Not supported in C and you will have to use the longer naming convention.
 * <BR><BR>So far this is what is supported:
 *  <OL>
 *  <LI><B>GPIO</B> on all available bank GPIO pins</LI>
 *  <LI><B>PWM</B> on all enabled pwm pins (Enable via device tree editor)</LI>
 *  <LI><B>FAKE PWM</B> on all available bank GPIO pins</LI>
 *  <LI><B>ACCEL</B> raw data and calibrated data with custom poll rates</LI>
 *  <LI><B>MAGNO</B> same as accel just a magnetometer (Magno for short)</LI>
 *  <LI><B>GYRO</B> same as accel just gyroscope</LI>
 *  <LI><B>TEMP</B> The I2C temperature brick sensor plugin</LI>
 *  <LI><B>BUILTIN</B> Control over builtin tools like disabling the m4 core and setting graphics options (Specific to Udoo Neo)</LI>
 *  <LI><B>SERVO</B> Currently experiemental but should work on any GPIO pin to write between 0 and 180 degrees on a servo</LI>
 *  </OL>
 *  <BR>
 * The reason for this library to be written in C and have C++ support (using extern), was mainly for the speed
 * Of the compiled low level languages and they're relatively easy to port up into higher level languages. That's
 * Why I need your help to manage and port this fast base into higher languages. I have too many other projects to
 * Work on and this was initially just made for a personal project. Then realized why not just add some documentation
 * And make it a library. To install just scroll down to the installation section.
 *
 * <BR>
 *  \section install_sec Installation
 * <BR><H3>Easy Installation</H3><BR>
 * The installation for NEOC is extremely easy. For the quick installation method just copy and paste the below command<BR>
 * <CODE>wget -q -O - https://raw.githubusercontent.com/smerkousdavid/NEOC.GPIO/install.sh | bash</CODE><BR>
 * <H3>Full Installation</H3><BR>
 * 
 *
 *  \section sec License
 * <BR>This Software may be used commercially if credit is provided to original owner in private source<BR><BR>
 * <TABLE STYLE="width: 50%;"><TR><TD><I>
 * <B>NEOC</B> (libneo) is free software: you can redistribute it and/or modify<BR>
 * &nbsp;&nbsp;&nbsp;&nbsp;it under the terms of the GNU General Public License as published by<BR>
 * &nbsp;&nbsp;&nbsp;&nbsp;the Free Software Foundation, either version 3 of the License, or<BR>
 * &nbsp;&nbsp;&nbsp;&nbsp;(at your option) any later version.<BR>
 * <BR>
 * <B>NEOC</B> (libneo) is distributed in the hope that it will be useful,<BR>      
 * &nbsp;&nbsp;&nbsp;&nbsp;but WITHOUT ANY WARRANTY; without even the implied warranty of<BR>
 * &nbsp;&nbsp;&nbsp;&nbsp;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<BR>   
 * &nbsp;&nbsp;&nbsp;&nbsp;GNU General Public License for more details.<BR>    
 *                                                                       
 * You should have received a copy of the <B>GNU General Public License</B><BR>     
 * &nbsp;&nbsp;&nbsp;&nbsp;along with this program.  If not, see http://www.gnu.org/licenses <BR>
 * </I></TD></TR></TABLE>
 */


#ifndef NEOC_H
#define NEOC_H

//Cpp included in header so the users wouldn't have to include two separate headers and
//Not have to use two different compiled libraries
#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN_SKIP

#define GPIOPORTSL 48
#define PWMPORTSL 7
#define ANALOGPORTSL 5
#define ANALOGSCALEL 2

#define EXPORTPATH "/sys/class/gpio/export"
#define GPIOPATH "/sys/class/gpio/gpio"
#define VALUEPATH "/value"
#define DIRECTIONPATH "/direction"

#define PWMEXPORTPATH "/sys/class/pwm/pwmchip0/export"
#define PWMPATH "/sys/class/pwm/pwmchip0/pwm"
#define PWMPERIOD "/period"
#define PWMDUTY "/duty_cycle"
#define PWMENABLE "/enable"

#define MAXFAKEPWM 47
#define PWMMUTEXRATE 0.00010f

#define ANALOGPATHP "/sys/bus/iio/devices/iio\:device"
#define ANALOGBASEP ((const char *) "/in_voltage")
#define ANALOGRAWP "_raw"
#define ANALOGSCALEP "_scale"
#define ANALOGHIGH 4095
#define ANALOGLOW 0
#define ANALOGBIT 12


#define LEDPATH "/sys/class/leds/led0/brightness"

#endif

//To scale the analog based on board specs set to true
//Auto find scale will be done
//#define SCALEANALOG

#define NEO_OK 0
#define NEO_FAIL -1
#define NEO_PIN_ERROR -2
#define NEO_PERIOD_ERROR -3
#define NEO_DUTY_ERROR -4
#define NEO_UNUSABLE_ERROR -5
#define NEO_EXPORT_ERROR -6
#define NEO_UNUSABLE_EXPORT_ERROR -7
#define NEO_DIR_ERROR -8
#define NEO_READ_ERROR -9
#define NEO_SCALE_ERROR -10
#define NEO_LED_STATE_ERROR -11

#ifndef DOXYGEN_SKIP

#define ACCELBASE "/sys/class/misc/FreescaleAccelerometer/"
#define ACCELENABLE ACCELBASE"enable"
#define ACCELDATA ACCELBASE"data"
#define ACCELPOLLP ACCELBASE"poll_delay"
#define ACCELPOLL 20

#define GYROBASE "/sys/class/misc/FreescaleGyroscope/"
#define GYROENABLE GYROBASE"enable"
#define GYRODATA GYROBASE"data"
#define GYROPOLLP GYROBASE"poll_delay"
#define GYROPOLL 20

#define MAGNOBASE "/sys/class/misc/FreescaleMagnetometer/"
#define MAGNOENABLE MAGNOBASE"enable"
#define MAGNODATA MAGNOBASE"data"
#define MAGNOPOLLP MAGNOBASE"poll_delay"
#define MAGNOPOLL 20

#define TEMPINIT "lm75 0x48"
#define TEMPPATH "/sys/class/i2c-dev/i2c-1/device/new_device"
#define TEMPREAD "/sys/class/i2c-dev/i2c-1/device/1-0048/temp1_input"

#endif

#define OUTPUT 1
#define INPUT 0
#define HIGH 1
#define LOW 0

#ifndef DOXYGEN_SKIP

#include <string.h>
#include <stdio.h>


#define exportL strlen(EXPORTPATH)
#define gpioL  strlen(GPIOPATH)
#define valueL strlen(VALUEPATH)
#define directionL strlen(DIRECTIONPATH)
#define pwmexportL strlen(PWMEXPORTPATH)
#define pwmL strlen(PWMPATH)
#define pwmPeriod strlen(PWMPERIOD)
#define pwmDuty strlen(PWMDUTY)
#define pwmEnable strlen(PWMENABLE)
#define analogL strlen(ANALOGPATHP)
#define analogBL strlen(ANALOGBASEP)
#define analogRL strlen(ANALOGRAWP)
#define analogSL strlen(ANALOGSCALEP)

#endif

extern const char * const GPIOPORTS[];
extern unsigned char USABLEGPIO[];
extern const char * const PWMPORTS[];
extern unsigned char USABLEPWM[];
extern const char * const ANALOGPORTS[][2];
extern unsigned char USABLEANALOG[];
extern float ANALOGSCALE[];

float neo_re_map(float, float, float, float, float);
void neo_check_root(char const *);
int neo_enable_m4();
int neo_disable_m4();
int neo_screen_set_lvds();
int neo_screen_set_hdmi();

//extern unsigned int neo_pwm_period = 2040816;
//extern unsigned int neo_pwm_duty = 50;

int neo_gpio_init();
int neo_gpio_pin_mode(int, int);
int neo_gpio_digital_write_no_safety(int*, int);
int neo_gpio_digital_write(int, int);
int neo_gpio_digital_read(int);
int neo_gpio_free();

int neo_pwm_init();

#ifndef DOXYGEN_SKIP

void neo_sync_pwm(void*, int*, int*, int*, int*);
void *pwmManager(void*);

#endif

int neo_fake_pwm_init();
int neo_fake_pwm_write_period(int, int, int);
int neo_fake_pwm_write(int, int);
int neo_pwm_set_period(int, int);
int neo_pwm_set_period_all(int);
int neo_pwm_write(int, int);
int neo_pwm_free();

int neo_analog_init();
float neo_analog_read(int);
int neo_analog_free();

int neo_temp_init();
int neo_temp_read();
int neo_temp_free();

int neo_led_init();
int neo_led_set(int);
int neo_led_on();
int neo_led_off();

#ifdef __cplusplus
}

class Analog {
	public:
		Analog(int port, bool release = true) {
			Analog::init(true);
			_held = port;
			_in_use += 1;

			Analog::_release = release;
		}

		~Analog() {
			_in_use -= 1;

			if(_in_use == 0 && Analog::_release) Analog::free(false);
		}

		static bool init(bool throws) {
			int ret = neo_analog_init();
			return ret == NEO_OK;
		}

		static bool init() {
			return Analog::init(false);
		}

		static bool free(bool throws) {
			int ret = neo_analog_free();
			return ret == NEO_OK;
		}

		static bool free() {
			return Analog::free(false);
		}

		static float read(int port) {
			float ret = neo_analog_read(port);
			return ret;
		}

		float read() {
			Analog::read(_held);
		}

	private:
		int _held;
		static short _in_use;
		static bool _release;
};

short Analog::_in_use = 0;
bool Analog::_release = true;

class Gpio {
	public:
		Gpio();
		void wilma(int);
	private:
    	int a_;
};
#endif //__cplusplus

#endif //NEO_HEAD
