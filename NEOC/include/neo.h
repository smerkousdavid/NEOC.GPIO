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

///@brief Returned on success or completion
#define NEO_OK 0

///@brief Returned on generic or unknown fail
#define NEO_FAIL -1

///@brief When pin is not within correct range (gpio that's 0 - 47)
#define NEO_PIN_ERROR -2

///@brief When the period is not valid (keep it between infinite hertz 0 and 1 Hertz)
#define NEO_PERIOD_ERROR -3

///@brief When the Duty cycle isn't valid, correct is between 0 and 255
#define NEO_DUTY_ERROR -4

///@brief When the pin is already being used, wasn't initialized or isn't plain working
#define NEO_UNUSABLE_ERROR -5

///@brief When it failed to initialize one or more pins 
#define NEO_EXPORT_ERROR -6

///@brief When it failed to initialize one or more pins because they don't exist
#define NEO_UNUSABLE_EXPORT_ERROR -7

///@brief When the direction is different from either (INPUT) 0 or (OUTPUT) 1
#define NEO_DIR_ERROR -8

///@brief When the Analog or Gpio pin don't have root permission or the kernel broke
#define NEO_READ_ERROR -9

///@brief When the Analog scaler decided to kaput and not scale analog, don't blame the programmer
#define NEO_SCALE_ERROR -10

///@brief When failed to set the builtin LED state
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

///@brief To set the GPIO pin to output mode (Readable format)
#define OUTPUT 1

///@brief To set the GPIO pin to input mode (Readable format)
#define INPUT 0

///@brief To set the GPIO pin state to HIGH aka on
#define HIGH 1

///@brief To set the GPIO or anything pin to state LOW aka off
#define LOW 0

#ifndef DOXYGEN_SKIP

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

extern const char * const GPIOPORTS[];
extern unsigned char USABLEGPIO[];
extern const char * const PWMPORTS[];
extern unsigned char USABLEPWM[];
extern const char * const ANALOGPORTS[][2];
extern unsigned char USABLEANALOG[];
extern float ANALOGSCALE[];
extern unsigned char neo_exit_set;

#endif

float neo_re_map(float, float, float, float, float);
void neo_check_root(char const *);
int neo_check_root_return();
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
float neo_analog_read_raw(int);
int neo_analog_free();

int neo_temp_init();
int neo_temp_read();
int neo_temp_free();

int neo_led_init();
int neo_led_set(int);
int neo_led_on();
int neo_led_off();
void neo_free_all();

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
 *  \section pwm Analog Examples
 *   <DIV CLASS="container"> 
 *     <UL>
 *       <LI><H3><A CLASS="examplelist" HREF="exampleanalog.html">Analog Read</A></H3></LI>
 *     </UL>
 *   </DIV>
 */



#ifdef __cplusplus
}

//Cpp included in header so the users wouldn't have to include two separate headers and
//Not have to use two different compiled libraries
#include "neoerror.h" 
#include <string>
#include <cstring>
#include <iostream>

namespace neo {

/**
 * @brief Arduino based linear remapping 
 *
 * This will remap any types combined (Number types) To return a linearly mapped value
 * reason for creation, because people on Arduino use it so much. Why not here also.
 * USAGE:
 * \code{.cpp}
 * float scaled = neo::map<float>(50, 0, 100, 0, 1000);
 * \endcode
 * 
 * The results of the code should be 500 because it was scaled 10 times.
 * @param value The input value you want to scale
 * @param inMin The minimum value the input can be
 * @param inMax The maximum value the input can be
 * @param outMin The scale value to be the new minimum
 * @param outMax The scale value to be the new maximum
 *
 * @return The provided type of the new scaled value
 */
template<typename T, typename V, typename M, typename MA, typename MI, typename MX>
T map(V value, M inMin, MA inMax, MI outMin, MX outMax) {
	return static_cast<T>(neo_re_map(static_cast<float>(value),
		static_cast<float>(inMin), static_cast<float>(inMax),
		static_cast<float>(outMin), static_cast<float>(outMax)));
}

/**
 * @brief Checks for root permission
 *
 * Some functions and aspect of the neo library REQUIRE root permission. This
 * can be easily done with just checking the uid that the program was started in
 * This will throw a neo::error::NotRootError when it failed to load root
 *
 * @param saying The const char array of what to say when failed to get root
 *
 * @note This won't automatically ask for a password, you either have to restart the program or setuid
 */
void checkRoot(const char * saying) {
	int checked = neo_check_root_return();
 
	if(checked != NEO_OK) {
		throw error::NotRootError(saying);
	}
}


/**
 * @brief Enables or Disables the m4
 *
 * This function will either enable (enabled = true) or disable (enabled = false)
 * the arduino m4 core. This will prevent the arduino sketch to be loaded on m4 reset or boot.
 * Also allow the A9 (Linux side) to use more of the pins, such as Analog
 *
 * @param enabled True to enable the core or False to disable the core
 * @return a bool if it was able to change the m4 core (false means failure)
 */
bool setM4(bool enabled) {
	checkRoot("Changing the state of the M4 requires root permission!"); //Will throw NotRootError
	return (((enabled) ? neo_enable_m4() : neo_disable_m4()) == NEO_OK);
}

/**The core enabling flag aka true*/
const bool ENABLED = true;

/**The core disabling flag aka false*/
const bool DISABLED = false;

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
		 * @param throwing Wether to throw erros like PinError or just surpress them (false to surpress) (default: true)
		 * 
		 * @note The m4 core must be disabled before using this Analog class @see neo_disable_m4()
		 */
		Analog(int port, bool release = false, bool throwing = true) {
			Analog::init(_throwing); //Use static instance
			_held = port;
			_in_use += 1; //Update usage count
			_throwing = throwing;

			Analog::_release = release; //If any release are false all are
		}

		/**
		 * @brief Analog release
		 * 
		 * This will attempt to release the objects, if specified in the constructor that
		 * when all objects are out of scope and destroyed to release all of the pins from the program
		 * 
		 * @note Do not try to mix Object oriented and static usage of any Neo Object it screws it up
		 * @note When attempting to use both static Analog::read(int) and obj.read() make sure you use Analog obj(<pin>, false) to not release on 0 objects
		 */
		~Analog() {
			_in_use -= 1;

			if(_in_use == 0 && Analog::_release) Analog::free();
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
				neo::error::Handler(ret, -1, 0, ANALOGPORTSL, 0, "Analog", "Failed to Init");
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
				neo::error::Handler(ret, -1, 0, ANALOGPORTSL, 0, "Analog", "Failed to Release");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static RAW reading from port
		 *
		 * This will read from the analog pin returning between 0 and 4095 and throw an exception if it failed to read from
		 * the pin. This one is scaled to be the actual current recieved voltage
		 *
		 * @return A float with the current voltage raw resolution
		 * @param port The port to statically read from
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 */
		static float readRaw(int port, bool throws = true) {
			float ret = neo_analog_read_raw(port);
			if(throws && static_cast<int>(ret) != NEO_OK) {
				neo::error::Handler(ret, port, 0, ANALOGPORTSL, 0, "Analog", "Failed to Reading RAW from Pin");
			}
			return (ret <= 0.0f) ? 0.0f : ret;
		}

		/**
		 * @brief Static reading from port
		 *
		 * This will read from the analog pin and throw an exception if it failed to read from
		 * the pin. This one is scaled to be the actual current recieved voltage. Between 0v and 3.3v
		 *
		 * @return A float with the current voltage raw resolution
		 * @param port The port to statically read from
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 */
		static float read(int port, bool throws = true) {
			float ret = neo_analog_read(port);
			if(throws && static_cast<int>(ret) != NEO_OK) {
				neo::error::Handler(ret, port, 0, ANALOGPORTSL, 0, "Analog", "Failed to Reading Voltage from Pin");
			}
			return (ret <= 0.0f) ? 0.0f : ret;
		}
		
		/**
		 * @brief Object read Raw (0 - 4095) from initialized port
		 *
		 * This will read from the preset port of the object, with Raw data. Warning this may throw
		 * errors when failing unless you specify in the constructor to not throw errors
		 *
		 * @return A float with the current voltage raw resolution
		 */
		float readRaw() {
			return Analog::readRaw(_held, _throwing);
		}
		
		/**
		 * @brief Object read from initialized port
		 *
		 * This will read from the preset port of the object. Warning this may throw
		 * errors when failing unless you specify in the constructor to not throw errors
		 *
		 * 
		 *
		 * @return A float with the current voltage (0v to 3.3v)
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
bool Analog::_release = false; //Set to automatically release

/** @class Gpio neo.h
 * @brief The gpio class that handles all General Pin Input and Output
 * 
 * This class is a helper to use the General Pin Input and Output. 
 * Just remember it's faster to use the C methods statically rather than
 * the objects and new instances. 
 *
 * <BR>Here is the example usage of the class:
 * \code{.cpp}
 * 
 * int main() {
 *   Gpio led(13); 
 *   led.setOut();
 *   led.on();
 *   return 0;
 * }
 * \endcode
 * <BR>
 * 
 * @note The m4 core DOESN'T have to be disabled, you just have to be very careful about what pins you use
 */
class Gpio {
	public:
		/**
		 * @brief Gpio constructor and initializer
		 *
		 * This initializes the Gpio class in the backend this calls the C function
		 * neo_gpio_init() that method is static as such Gpio::init(); 
		 *
		 * @param port The analog port (0 to 47) to load the object in
		 * @param release When no more instances of the object are detected auto release (default: true)
		 * @param throwing Wether to throw erros like PinError or just surpress them (false to surpress) (default: true)
		 * 
		 * @see neo_gpio_init()
		 * @see neo_gpio_pin_mode()
		 * @see neo_gpio_digital_write()
		 * @see neo_gpio_digital_read()
		 * @see neo_gpio_free() 
		 *
		 * @note Careful to not use the same port as output as the m4 (arduino core)
		 */
		Gpio(int port, bool release = false, bool throwing = true) {
			Gpio::init(_throwing); //Use static instance
			_held = port;
			_throwing = throwing;
			
			Gpio::_in_use += 1; //Update usage count
			Gpio::_release = release; //If any release are false all are
		}

		/**
		 * @brief Gpio pin object deconstructor and pin release
		 * 
		 * This will attempt to release the objects, if specified in the constructor that
		 * when all objects are out of scope and destroyed to release all of the pins from the program
		 * 
		 * @note Do not try to mix Object oriented and static usage of any Neo Object it screws it up
		 * @note When attempting to use both static Gpio::...() and obj....() make sure you use Gpio obj(<pin>, false) to not release on 0 objects
		 * @note When using static :: and object . at the same time, you use false on release on the object constructor and must call Gpio::free() at the end of the program
		 */
		~Gpio() {
			Gpio::_in_use -= 1;

			if(Gpio::_in_use == 0 && Gpio::_release) Gpio::free();
		}

		/**
		 * @brief Static initializer
		 *
		 * A functions that wraps the C neo_gpio_init function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 * 
		 */
		static bool init(bool throws = false) {
			int ret = neo_gpio_init();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, GPIOPORTSL, 0, "Gpio", "Failed to Init");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static de-initializer
		 *
		 * A functions that wraps the C neo_gpio_free function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 */
		static bool free(bool throws = false) {
			int ret = neo_gpio_free();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, GPIOPORTSL, 0, "Gpio", "Failed to Release");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static reading from port
		 *
		 * This will read from the gpio pin and throw an exception if it failed to read from
		 * the pin. The returned value should only ever be LOW or HIGH. Nothing will be returned when
		 * an error is thrown, if throws is false then it's always LOW;
		 *
		 * @return An int with either a HIGH (1) or LOW (0) value  
		 * @param port The port to statically read from
		 * @param throws Optional value to throw if there is an error
		 */
		static int read(int port, bool throws = true) {
			int ret = neo_gpio_digital_read(port);
			if(throws && (ret != HIGH || ret != LOW)) {
				neo::error::Handler(ret, port, 0, GPIOPORTSL, 0, "Gpio", "Failed to reading from Gpio Pin");
			}
			return (ret <= 0) ? LOW : HIGH;
		}
		
		/**
		 * @brief Object read from initialized port
		 *
		 * This will read from the preset port of the object. Warning this may throw
		 * errors when failing unless you specify in the constructor to not throw errors
		 *
		 * @return either a LOW or a HIGH 
		 */
		int read() {
			return Gpio::read(_held, _throwing);
		}
		
		/**
		 * @brief Static writing to port
		 *
		 * This will write a value to the gpio pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param value the value to write either HIGH (1) or LOW (0) (An error will be thrown otherwise)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool write(int port, int value, bool throws = true) {
			int ret = neo_gpio_digital_write(port, value);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, GPIOPORTSL, 0, "Gpio", "Failed to writing to Gpio Pin");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Writing to selected object port
		 *
		 * This will write a value to the gpio pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded 
		 * @param value the value to write either HIGH (1) or LOW (0) (An error will be thrown otherwise)
		 */
		bool write(int value) {
			return Gpio::write(_held, value, _throwing);
		}
		
		/**
		 * @brief Turning on the selected pin
		 *
		 * Attempt to write HIGH on the selected object port/pin
		 *
		 * @return A boolean if the operation succeded
		 */
		bool on() {
			return this->write(HIGH);
		}
		
		/**
		 * @brief Turning off the selected pin
		 *
		 * Attempt to write LOW on the selected port/pin
		 *
		 * @return A boolean if the operation succeded
		 */
		bool off() {
			return this->write(LOW);		
		}
		
		/**
		 * @brief Static setting pin mode
		 *
		 * This will attempt to set the pin direction. Will throw error if pin or direction is wrong
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param dir the value to set the direction either OUTPUT (1) or INPUT (0)
		 * @param throws Optional value to throw if there is an error (default: true)
		 *
		 * @warning Do not set the same pin on the m4 to OUTPUT!
		 */
		static bool pinMode(int port, int dir, bool throws = true) {
			int ret = neo_gpio_pin_mode(port, dir);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, GPIOPORTSL, 0, "Gpio", "Failed to set direction of Gpio Pin");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Setting the direction of the object port
		 *
		 * This will attempt to set the direction of the pin and throw an exception if it failed to set it.
		 *
		 * @return A boolean if the operation succeded 
		 * @param dir the value to set the direction either OUTPUT (1) or INPUT (0)
		 *
		 * @warning Do not set the same pin on the m4 to OUTPUT!
		 */
		bool setDir(int dir) {
			return Gpio::pinMode(_held, dir, _throwing);
		}
		
		/**
		 * @brief Setting the current pin to OUTPUT
		 *
		 * Attempt to set the pin to OUTPUT mode
		 *
		 * @return A boolean if the operation succeded
		 *
		 * @warning Do not set the same pin on the m4 to OUTPUT!
		 */
		bool setOut() {
			return this->setDir(OUTPUT);
		}
		
		/**
		 * @brief Setting the current pin to INPUT
		 *
		 * Attempt to set the pin to INPUT mode
		 *
		 * @return A boolean if the operation succeded
		 */
		bool setIn() {
			return this->setDir(INPUT);	
		}

	private:
		int _held; //Current pin to use
		bool _throwing;
		static short _in_use; //Global object usage count
		static bool _release; //Global to release on no object count
};

short Gpio::_in_use = 0; //Set no object counts
bool Gpio::_release = false; //Set to automatically release

/** @class PWM neo.h
 * @brief The PWM class that handles all PWM controls
 * 
 * This class helps out with controlling the Real PWM controllers
 * these use real timers and clocks to give really nice PWM modules and outputs.
 * Only problem is that they're a pain to map out. So that part can get a bit tricky
 * check out the example PWM Real page for for information
 *
 * <BR>Here is the example usage of the class:
 * \code{.cpp}
 * 
 * int main() {
 *   PWM pwm(0); //PWM 5 if mapped correctly 
 *   pwm.write(127); //About half duty cycle
 *   return 0;
 * }
 *
 * \endcode
 * <BR>
 * 
 * @note Be careful to not use the same pins as Gpio
 */
class PWM {
	public:
		/**
		 * @brief PWM constructor and initializer
		 *
		 * This initializes the PWM class in the backend this calls the C function
		 * neo_pwm_init() that method is static as such PWM::init(); 
		 *
		 * @param port The PWM port (0 to 7) to load the object in
		 * @param release When no more instances of the object are detected auto release (default: true)
		 * @param throwing Wether to throw erros like PinError or just surpress them (false to surpress) (default: true)
		 * 
		 * @see neo_pwm_init()
		 * @see neo_pwm_set_period()
		 * @see neo_pwm_set_period_all()
		 * @see neo_pwm_write()
		 * @see neo_pwm_free() 
		 *
		 * @note Careful to not use the same port as output as the m4 (arduino core)
		 */
		PWM(int port, bool release = false, bool throwing = true) {
			PWM::init(); //Use static instance
			_held = port;
			_throwing = throwing;
			
			PWM::_in_use += 1; //Update usage count
			PWM::_release = release; //If any release are false all are
		}

		/**
		 * @brief PWM pin object deconstructor and pin release
		 * 
		 * This will attempt to release the objects, if specified in the constructor that
		 * when all objects are out of scope and destroyed to release all of the pins from the program
		 * 
		 * @note Do not try to mix Object oriented and static usage of any Neo Object it screws it up
		 * @note When attempting to use both static PWM::...() and obj....() make sure you use PWM obj(<pin>, false) to not release on 0 objects
		 * @note When using static :: and object . at the same time, you use false on release on the object constructor and must call PWM::free() at the end of the program
		 */
		~PWM() {
			PWM::_in_use -= 1;

			if(PWM::_in_use == 0 && PWM::_release) PWM::free();
		}

		/**
		 * @brief Static initializer
		 *
		 * A functions that wraps the C neo_pwm_init function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 * 
		 */
		static bool init(bool throws = false) {
			int ret = neo_pwm_init();
			if(throws && (ret != NEO_OK && ret != NEO_UNUSABLE_ERROR)) {
				neo::error::Handler(ret, -1, 0, PWMPORTSL, 0, "PWM", "Failed to Init");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static de-initializer
		 *
		 * A functions that wraps the C neo_pwm_free function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 */
		static bool free(bool throws = false) {
			int ret = neo_pwm_free();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, PWMPORTSL, 0, "PWM", "Failed to Release");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Static writing to port
		 *
		 * This will write a duty cycle to the pwm pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param duty the duty cycle to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool write(int port, int duty, bool throws = true) {
			int ret = neo_pwm_write(port, duty);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, 255, duty, "PWM", "Failed to Writing to PWM Pin");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Writing to selected object port
		 *
		 * This will write a duty cycle to the pwm pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded 
		 * @param duty the value to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 */
		bool write(int duty) {
			return PWM::write(_held, duty, _throwing);
		}
		
		/**
		 * @brief Static setting PWM period
		 *
		 * This will attempt to update the PWM period on a SINGLE PIN
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param period The period of the pin between 1 and 100000000 (in nano seconds)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool setPeriod(int port, int period, bool throws = true) {
			int ret = neo_pwm_set_period(port, period);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, 10000000, period, "PWM", "Failed to setting period on PWM Pin");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Static setting of all PWM pin periods
		 *
		 * This will attempt to update the PWM period on a ALL the PWM pins. This
		 * is essentially just a for loop, but to make it easier for the lazy people.
		 *
		 * @return A boolean if the operation succeded or not
		 * @param period The period of the pin between 1 and 100000000 (in nano seconds)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool setAllPeriods(int period, bool throws = true) {
			int ret = neo_pwm_set_period_all(period);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, 10000000, period, "PWM", "Failed to setting period on ALL PWM Pins");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Setting PWM period on selected object pin
		 *
		 * This will attempt to update the PWM period on the currently selected pin
		 *
		 * @return A boolean if the operation succeded or not
		 * @param period The period of the pin between 1 and 100000000 (in nano seconds)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		bool setPeriod(int period) {
			return PWM::setPeriod(_held, period, _throwing);
		}

	private:
		int _held; //Current pin to use
		bool _throwing;
		static short _in_use; //Global object usage count
		static bool _release; //Global to release on no object count
};

short PWM::_in_use = 0; //Set no object counts
bool PWM::_release = false; //Set to automatically release

/** @class FakePWM neo.h
 * @brief The FakePWM class that handles all FakePWM controls (GPIO)
 * 
 * This class helps out getting quick and dirty PWM on any Gpio port with the Udoo
 * Neo. I'm going to say this right now, the PWM output on the pins are not clean at all
 * each period cycle might be different. I just have to say, it works... It's going to do
 * It's job easily. Since you are limited on the amount of Real PWM I would recommend using
 * FakePWM on things like LEDS where precision doesn't matter.
 *
 * <BR>Here is the example usage of the class:
 * \code{.cpp}
 * 
 * int main() {
 *   FakePWM pwm(13); //Gpio pin 13 
 *   pwm.write(127); //About half duty cycle
 *   return 0; //Auto free on exit
 * }
 *
 * \endcode
 * <BR>
 */
class FakePWM {
	public:
		/**
		 * @brief PWM constructor and initializer
		 *
		 * This initializes the PWM class in the back end this calls the C function
		 * neo_fake_pwm_init() that method is static as such FakePWM::init(); 
		 *
		 * @param port The Gpio port to simulate on (0 to 47) to load the object in
		 * @param release When no more instances of the object are detected auto release (default: false)
		 * @param throwing Wether to throw errors like PinError or just surpress them (false to surpress) (default: true)
		 * 
		 * @see neo_fake_pwm_init()
		 * @see neo_fake_pwm_write_period()
		 * @see neo_fake_pwm_write()
		 * @see neo_gpio_free() //That's right learn to release correctly
		 *
		 * @note Careful to not use the same port as output as the m4 (arduino core)
		 */
		FakePWM(int port, bool release = false, bool throwing = true) {
			FakePWM::init(); //Use static instance
			_held = port;
			_throwing = throwing;
			_period = 20408; //~49KHz
			
			FakePWM::_in_use += 1; //Update usage count
			FakePWM::_release = release; //If any release are false all are
		}

		/**
		 * @brief FakePWM pin object deconstructor and pin release
		 * 
		 * This will attempt to release the objects, if specified in the constructor that
		 * when all objects are out of scope and destroyed to release all of the pins from the program
		 *
		 * @note Free needs no longer to be called since the method is called when the program exits
		 * @note You may call FakePWM::free(); Whenever you wish, just remember it's auto handled now
		 */
		~FakePWM() {
			FakePWM::_in_use -= 1;

			if(FakePWM::_in_use == 0 && FakePWM::_release) FakePWM::free();
		}

		/**
		 * @brief Static initializer
		 *
		 * A functions that wraps the C neo_fake_pwm_init function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 * 
		 */
		static bool init(bool throws = false) {
			int ret = neo_fake_pwm_init();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, GPIOPORTSL, 0, "FakePWM", "Failed to Init");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static de-initializer
		 *
		 * A functions that wraps the C neo_gpio_free function with some exception throwing
		 * and nice namespace conventioning.
		 *
		 * @param throws A boolean to indicate if the object should throw an error when it fails
		 *
		 * @warning This function will free all other gpio as well!!
		 * @note When called the backend neo_gpio_free(); is also called
		 */
		static bool free(bool throws = false) {
			int ret = neo_gpio_free();
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, -1, 0, GPIOPORTSL, 0, "FakePWM", "Failed to Release");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Static writing to gpio port
		 *
		 * This will write a duty cycle to the pwm pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param duty the duty cycle to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool writeDuty(int port, int duty, bool throws = true) {
			int ret = neo_fake_pwm_write(port, duty);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, 255, duty, "FakePWM", "Failed to Writing to FakePWM Pin");
			}
			return ret == NEO_OK;
		}
		
		/**
		 * @brief Static writing to gpio port with temporary new period
		 *
		 * This will write a duty cycle and period to the gpio fake pwm pin and throw an exception if it failed to write/update.
		 *
		 * @return A boolean if the operation succeded or not
		 * @param port The port to statically write to
		 * @param duty The duty cycle to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 * @param period The period of the pin between 1 and 100000000 (in nano seconds)
		 * @param throws Optional value to throw if there is an error (default: true)
		 */
		static bool writePeriod(int port, int duty, int period, bool throws = true) {
			int ret = neo_fake_pwm_write_period(port, period, duty);
			if(throws && ret != NEO_OK) {
				neo::error::Handler(ret, port, 0, 255, duty, "FakePWM", "Failed to Writing to FakePWM Pin");
			}
			return ret == NEO_OK;
		}

		/**
		 * @brief Writing to selected object port with custom period
		 *
		 * This will write a duty cycle to the gpio fake pwm pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded 
		 * @param period The period of the pin between 1 and 100000000 (in nano seconds)
		 * @param duty The value to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 * @param setDefault a boolean to overwrite the current default period(Just for this pin) (default: false)
		 */
		bool write(int duty, int period, bool setDefault = false) {
			if(setDefault) {
				if(period > 0 && period < 1000000000) this->_period = period;
			}
			return FakePWM::writePeriod(_held, duty, period, _throwing);
		}
	
		/**
		 * @brief Writing to selected object port
		 *
		 * This will write a duty cycle to the gpio fake pwm pin and throw an exception if it failed to write.
		 *
		 * @return A boolean if the operation succeded 
		 * @param duty The value to write between 0 (off) and 255 (full) (An error will be thrown otherwise)
		 */
		bool write(int duty) {
			return FakePWM::writePeriod(_held, duty, _period, _throwing);
		}
		
		/**
		 * @brief Setting PWM period on selected object pin
		 *
		 * This will attempt to update the PWM period on the currently selected pin
		 *
		 * @return A boolean if the operation succeded or not
		 * @param the period of the pin between 1 and 100000000 (in nano seconds)
		 */
		void setPeriod(int period) {
			this->_period = period;
		}

	private:
		int _held; //Current pin to use
		bool _throwing;
		int _period; //The period cycle
		static short _in_use; //Global object usage count
		static bool _release; //Global to release on no object count
};

short FakePWM::_in_use = 0; //Set no object counts
bool FakePWM::_release = false; //Set to automatically release


}
#endif //__cplusplus



#endif //NEO_HEAD
