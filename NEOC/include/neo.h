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
 *  \section intro Introduction
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
 * <BR><BR><BR>
 *  \subsection browsing Browsing Documentation
 *  This is all generated via Doxygen and some custom doxygen-bootstrap. I know it doens't look like the standard doxygen
 *  that's because the original doesn't look too great. Either than that, the navigation should be roughly the same.
 *  To view all the C++ classes and data structures (not available in C) click the data structures drop down and click
 *  Classes and Index for a alphabetical list of the classses and structures. For the C users I would recommed going to Files
 *  By pressing the Files button at the top and selecting neo.h to view all the available functions for C also usable in C++.
 *  All the source C code files should be documented and you can see all the warnings and arguments for each in the Files page.
 *  Almost.., there is Functions page that just lists out an entire list of documented functions and in what files they're located in.<BR>
 *  Finally there are some examples that could greatly benefit you. Try looking through the example codes and snippets, whenever you are curious
 *  about a specific method or function just hover over that function (until you see an underline) and click on it. The documented element
 *  will appear on the page. 
 *
 * <BR>
 *  \section install_sec Installation
 * <BR><H3>Easy Installation</H3>
 * The installation for NEOC is extremely easy. For the quick installation method just copy and paste the below command<BR>
 * \code{.sh} wget -q -O - https://raw.githubusercontent.com/smerkousdavid/NEOC.GPIO/master/install.sh | bash \endcode <BR>
 * <H3>Full Installation</H3>
 * <OL>
 * <LI>Clone and cd into the repo<BR>\code{.sh} git clone https://github.com/smerkousdavid/NEOC.GPIO && cd NEOC.GPIO/NEOC \endcode </LI>
 * <LI>Make the sources \code{.sh} make \endcode </LI>
 * <LI>Install the sources \code{.sh} sudo make install \endcode </LI>
 * <LI>Clean up \code{.sh} make clean \endcode </LI>
 * <LI><I>Optional:</I> Make latest documentation<BR> \code{.sh} sudo ./gendocs.sh \endcode </LI>
 * <LI>You're done!</LI>
 * </OL>
 *
 * <BR>
 *  \section usage Usage
 *  This is a simple gpio example to blink the led pin in C (inner bank pin 13)
 *  \code{.c}
 *  #include <neo.h>
 *  #include <unistd.h>
 * 
 *  //Main funciton
 *  int main() {
 *    neo_gpio_init(); //Must be called before using any other gpio pin function 
 *
 *    neo_gpio_pin_mode(13, OUTPUT); //Set pin 13 to output
 *
 *    int i;
 *
 *    //Loop ten times (Blink)
 *    for(i = 0; i < 10; i++) {
 *      neo_gpio_digital_write(13, HIGH); //Write HIGH to pin
 *      sleep(1); //Wait one second
 *      neo_gpio_digital_write(13, LOW); //Write LOW to pin
 *      sleep(1); //Wait one second
 *    }
 *
 *    neo_gpio_free(); //Should be called to release the pins from the program (Called at end of program)
 *  }
 *  \endcode
 * <BR>Here is the same example except using C++ classes
 * \code{.cpp}
 *  #include <neo.h>
 *  #include <chrono>
 *  #include <thread>
 * 
 *  using namespace neo; 
 *  using namespace std;
 *
 *  //Main function
 *  int main() {
 *      Gpio led(13); //Start pin 13 as a gpio pin
 *      led.setOut(); //Set the pin to OUTPUT      
 *
 *      for(int ind = 0; ind < 10; ind++)  {
 *         led.on();
 *         this_thread::sleep_for(chrono::milliseconds(1000));
 *         led.off();
 *         this_thread::sleep_for(chrono::milliseconds(1000));
 *      }
 *     //Auto release since the class releases the pin
 *  }
 * \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc inputfile.c -I/usr/include -lneo -o fileout.o
 *   chmod 755 fileout.o
 *   ./fileout.o
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ inputfile.cpp -I/usr/include -lneo -o fileout.o
 * \endcode
 * <BR>
 *  \section license License
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
int neo_gpio_digital_write(int, int);
int neo_gpio_digital_read(int);
int neo_gpio_free();

int neo_pwm_init();

#ifndef DOXYGEN_SKIP

int neo_gpio_digital_write_no_safety(int*, int);

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

/** \page examples Examples
 *  \tableofcontents
 *  All of the example pages below will show you how to run the code samples, installation is on the main page
 *  \section gpio Gpio Examples 
 *   <DIV CLASS="container"> 
 *     <UL>
 *       <LI><H3><A CLASS="examplelist" HREF="gpioblink.html">Blink</A></H3></LI>
 *       <LI><H3><A CLASS="examplelist" HREF="gpioread.html">Read</A></H3></LI>
 *     </UL>
 *   </DIV>
 *  \section pwm PWM Examples
 *   <DIV CLASS="container"> 
 *     <UL>
 *       <LI><H3><A CLASS="examplelist" HREF="examplepwm.html">Real Write</A></H3></LI>
 *       <LI><H3><A CLASS="examplelist" HREF="examplefakepwm.html">Fake Fade</A></H3></LI>
 *     </UL>
 *   </DIV>
 */



#ifdef __cplusplus
}

//Cpp included in header so the users wouldn't have to include two separate headers and
//Not have to use two different compiled libraries
#include "neoerror.h"

/** @class Analog neo.h
 * @brief The Cpp Analog class to read pins A0 - A5
 * 
 * This will read the current voltage from the analog pins
 * on A0 through A5. Here is the example usage of the class:
 * \code{.cpp}
 * 
 * int main() {
 *   Analog azero(0); 
 *   std::cout << "Analog (A0) Read: " << azero.read() << std::endl;
 *   return 0;
 * }
 * \endcode
 * <BR>
 * 
 * The m4 core must be disabled before running this method @see neo_disable_m4();
 */
class Analog {
	public:
		/**
		 * @brief Analog constructor and initializer
		 *
		 * This initializes the Analog class in the backend this calls the C function
		 * neo_analog_init() that method is static as such Analog::init(); 
		 *
		 * @param port The analog port (0 to 5) to load the object in
		 * @param release When no more instances of the object are detected auto release (default: true)
		 * 
		 * @note The m4 core must be disabled before using this Analog class @see neo_disable_m4()
		 */
		Analog(int port, bool release = true, bool throwing = true) {
			Analog::init(_throwing); //Use static instance
			_held = port;
			_in_use += 1; //Update usage count
			_throwing = throwing;

			Analog::_release = release; //If any release are false all are
		}

		/**
		 * @brief Analog release
		 * 
		 * This will attempt to release the objects, if there are no more Analog Objects in use
		 * then this will attempt to run Analog::free(); aka neo_analog_free(); To stop this construct object with
		 * \code{.cpp} Analog analog(<pin number>, false); //No release on deconstructor \endcode
		 * 
		 * @note Do not try to mix Object oriented and static usage of any Neo Object it screws it up
		 * @note When attempting to use both static Analog::read(int) and obj.read() make sure you use Analog obj(<pin>, false) to not release on 0 objects
		 */
		~Analog() {
			_in_use -= 1;

			if(_in_use == 0 && Analog::_release) Analog::free(_throwing);
		}

		/**
		 * @brief Static initializer
		 *
		 * A functions that wraps the C neo_analog_init function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 * 
		 * @note The m4 core must be disabled before init is called statically @see neo_disable_m4()
		 */
		static bool init(bool throws = false) {
			int ret = neo_analog_init();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, 5, 0, "Analog", "Failed to Init");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static de-initializer
		 *
		 * A functions that wraps the C neo_analog_free function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 */
		static bool free(bool throws = false) {
			int ret = neo_analog_free();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, 5, 0, "Analog", "Failed to Release");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static reading from port
		 *
		 * This will read from the analog pin and throw an exception if it failed to read from
		 * the pin.
		 *
		 * @return A float with the current voltage raw resolution
		 * @param port The port to statically read from
		 */
		static float read(int port, bool throws = false) {
			float ret = neo_analog_read(port);
			if(throws && static_cast<int>(ret) != NEO_OK) {
				neo::error::Handler(ret, port, 0, 5, 0, "Analog", "Failed to Read from Pin");
			}
			return ret;
		}

		/**
		 * @brief Object read from initialized port
		 *
		 * This will read from the preset port of the object. Warning this may throw
		 * errors when failing unless you specify in the constructor to not throw errors
		 *
		 * 
		 *
		 * @return A float with the current voltage raw resolution
		 */
		float read() {
			return Analog::read(_held, _throwing);
		}

	private:
		int _held; //Current pin to use
		bool _throwing;
		static short _in_use; //Global object usage count
		static bool _release; //Global to release on no object count
};

short Analog::_in_use = 0; //Set no object counts
bool Analog::_release = true; //Set to automatically release

class Gpio {
	public:
		Gpio();
		void wilma(int);
	private:
    	int a_;
};
#endif //__cplusplus

#endif //NEO_HEAD
