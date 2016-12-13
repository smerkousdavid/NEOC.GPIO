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
 * @file neoerror.h
 * @author David Smerkous
 * @date 11/27/2016
 * @brief Header file that includes all C++ throwables and errors
 *
 * This headers contains all the exceptions for the C++ methods of libneo
 * this is already included in neo.h, so there is no need to include this into
 * your main file. Although it never hurts if you do.
 * 
 * @see https://github.com/smerkousdavid/NEOC.GPIO
 * 
 */


#ifdef __cplusplus

#ifndef NEOERROR_H
#define NEOERROR_H

#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include <string>
#include <stdio.h>

//neo::error::XXXX...
namespace neo { namespace error {

	/** @class PinError neoerror.h
	 * @brief Thrown usually when the user inputs an invalid Pin Number 
	 *
	 * This exception is usually thrown when someone attempts to write to or
	 * read from an invalid port. Such as -1 on Gpio pin(-1); Will throw PinError
	 */
	class PinError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of PinError
		 *
		 * Example usage to throw a PinError
		 * \code{.cpp}
		 * throw PinError(-1, 0, 47);
		 * \endcode
		 *
		 * @param p The current pin selected (That's probably wrong)
		 * @param l The lowest pin available
		 * @param h The highest pin available
		 *
		 * @return A runtime_error
		 */
		PinError(int p, int l, int h) : std::runtime_error("PinError"), 
										pin(p), low(l), high(h) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of PinError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[200]; //Needs to be dynamic since it's out of scope arr
			sprintf(retArr, "PinError: Pin %d is not valid, valid options \
are %d <-> %d", pin, low, high);
			return retArr;
		}
	private:
		int pin, low, high;
	};

	/** @class PeriodError neoerror.h
	 * @brief Thrown usually when the user inputs an invalid Period Nano Second
	 *
	 * This exception is usually thrown when someone attempts to set an invalid
	 * period on the PWM methods. Valid are 1 -> 1000000000 nano seconds. The larger
	 * the number the slower the PWM module will pulse and switch from high to low.
	 * Negative numbers are not allowed.
	 */
	class PeriodError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of PeriodError
		 *
		 * Example usage to throw a PeriodError
		 * \code{.cpp}
		 * throw PeriodError(-1245494);
		 * \endcode
		 *
		 * @param p The wrongly selected period number 
		 *
		 * @return A runtime_error
		 */
		PeriodError(int p) : std::runtime_error("PeriodError"), period(p) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of PeriodError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "PeriodError: Period %d is not valid! Check \
documentation for valid options!", period);
			return retArr;
		}
	private:
		int period;
	};

	/** @class DutyError neoerror.h
	 * @brief Thrown usually when the user inputs an invalid Duty Cycle
	 *
	 * This exception is usually thrown when someone attempts to set an invalid
	 * Duty Cycle on the PWM methods. The valid range is from 0 aka LOW to 255
	 * aka 100% duty cycle always on. If outside of those ranges, this error will
	 * be thrown.
	 */
	class DutyError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of DutyError
		 *
		 * Example usage to throw a DutyError
		 * \code{.cpp}
		 * throw DutyError(300, 0, 255);
		 * \endcode
		 *
		 * @param d The wrongly selected Duty Cycle
		 * @param l The lowest value available
		 * @param h The highest value available
		 *
		 * @return A runtime_error
		 */
		DutyError(int d, int l, int h) : std::runtime_error("DutyError"), duty(d), low(l), high(h) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of DutyError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "DutyError: Duty cycle %d is not valid, valid \
options are %d <-> %d", duty, low, high);
			return retArr;
		}
	private:
		int duty, low, high;
	};

	/** @class DirError neoerror.h
	 * @brief Thrown usually when the user inputs an invalid Direction
	 *
	 * This exception is usually thrown when someone attempts to set an invalid
	 * Direction on the GPIO methods. The only two directions allowed are INPUT(0)
	 * and OUTPUT(1). Anything else will throw this error; 
	 */
	class DirError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of DirError
		 *
		 * Example usage to throw a DirError
		 * \code{.cpp}
		 * throw DirError(2);
		 * \endcode
		 *
		 * @param d The wrongly selected direction
		 *
		 * @return A runtime_error
		 */
		DirError(int d) : std::runtime_error("DirError"), direction(d) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of DirError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "DirError: Direction %d is not valid, \
valid options are INPUT (0) or OUTPUT (1)", direction);
			return retArr;
		}
	private:
		int direction;
	};

	/** @class UnusableError neoerror.h
	 * @brief Most commonly thrown error when the specified pin is unusable
	 *
	 * This one can be very problematic because it could be sometimes hard to
	 * find what the issue is. Most of the time it's because of these issues<BR>
	 * <UL>
	 *	<LI>PWM pin wasn't properly pinmuxed using device tree editor</LI>
	 *	<LI>The PWM pin isn't the same mapping as it is in the device tree editor
	 * <B>trust me it is usually not</B></LI>
	 *	<LI>GPIO is already using the same bank pin where PWM is trying to use</LI>
	 *  <LI>Same as above but vise versa with GPIO and PWM</LI>
	 *	<LI>You don't have suffice permissions, try running as root</LI>
	 *	<LI>The OS just plain won't let you allocate that pin, probably meaning
	 * you havent disabled the m4 core yet</LI>
	 * </UL><BR>
	 * As soon as you have tried fixing all of those issues or at least checking
	 * for them. Try emailing me at smerkousdavid@gmail.com to see if I can try to help
	 * you out. I understand this error can be extrememly annoying.
	 */
	class UnusableError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of UnusableError
		 *
		 * Example usage to throw a UnusableError
		 * \code{.cpp}
		 * throw UnusableError("PWM", 3); //3 wasn't pinmuxed
		 * \endcode
		 *
		 * @param w What system core are you using (Usually GPIO or PWM)
		 * @param p What pin on that core is trying to be allocated
		 *
		 * @return A runtime_error
		 */
		UnusableError(const char * w, int p) : std::runtime_error("Unusable\
Error"), whatType(w), pin(p) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of UnusableError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "UnusableError: (%s) -> Pin %d is not usable, make \
sure it was initialized and it's not being used by something else. \
Best thing to do is check documentation for reason", whatType, pin);
			return retArr;
		}
	private:
		const char *whatType;
		int pin;
	};

	/** @class InitError neoerror.h
	 * @brief Thrown when the program failed to allocate pins on the static init methods
	 *
	 * This is also a tricky one, this one will be thrown and if you catch it and still
	 * try to write to a pin, you will 95% be guaranteed that a UnusableError will be thrown
	 * since it failed to initialize in the first place. Expect to see this thrown a lot on
	 * PWM. That's fine, because of the way the PWM manager handles the allocation, there is
	 * no guess as to which pin is mapped to where, so it will try all the mappings. Almost
	 * never will you not get an InitError thrown on PWM, it's best to set the throw flag on PWM
	 * to off, if you don't want to put a try catch around it.
	 */
	class InitError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of UnusableError
		 *
		 * Example usage to throw a UnusableError
		 * \code{.cpp}
		 * throw InitError("ANALOG"); //The m4 core probably wasn't disabled
		 * \endcode
		 *
		 * @param w What core system is not usable usually ("ANALOG" or "PWM")
		 *
		 * @return A runtime_error
		 */
		InitError(const char * w) : std::runtime_error("InitError"), whatType(w) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of InitError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "InitError: %s is not usable, make sure it's not being used by \
something else and you are in root (for safety). Best thing to do is check documentation for reason", whatType);
			return retArr;
		}
	private:
		const char *whatType;
	};

	/** @class ReadWriteError neoerror.h
	 * @brief Thrown when for some dumb reason the program failed to write or read from a pin
	 *
	 * If you haven't recieved a UnusableError and still got a ReadWriteError. It usually means
	 * if you at the EXACT same time try to write to a pin, then flush (in the backend) flush
	 * the new settings. It will throw this error, either than that you should rarely ever see this.
	 */
	class ReadWriteError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of ReadWriteError
		 *
		 * Example usage to throw a ReadWriteError
		 * \code{.cpp}
		 * throw ReadWriteError("ANALOG", "Failed reading analog port"); 
		 * //Couldn't read Analog (try rebooting)
		 * \endcode
		 *
		 * @param w What core system is not usable usually ("ANALOG" or "GPIO")
		 * @param t What is the type of error, read or write error
		 *
		 * @return A runtime_error
		 */
		ReadWriteError(const char * w, const char *t) : std::runtime_error("ReadWriteError"), 
			whatType(w), typeError(t) {}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of ReadWriteError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "InitError: (%s) failed to %s. It's still usable just something failed \
to change pin state", whatType, typeError);
			return retArr;
		}
	private:
		const char *whatType, *typeError;
	};

	/** @class NotRootError neoerror.h
	 * @brief Thrown when the program isn't root and is trying to access a root function
	 *
	 * This is relatively self explanitory. When you are trying to access a root function
	 * such as the I2C temperature bricks or the Analog pins. The program will require root
	 * to read and write to these highely privileged pins
	 */
	class NotRootError : public std::runtime_error {
	public:
		/**
		 * @brief Constructor of NotRootError
		 *
		 * Example usage to throw a NotRootError
		 * \code{.cpp}
		 * throw NotRootError("You need root for analog reading!"); 
		 * //Couldn't read Analog (try running program as root)
		 * \endcode
		 *
		 * @param w What to say on what() when the program isn't root
		 * @return A runtime_error
		 */
		NotRootError(const char * w) : std::runtime_error("NotRootError"), whatType(w){}

		/**
		 * @brief The compiled response on catch of exception or Abort
		 *
		 * This is called when the coder catches the exception or is just plainly
		 * Thrown, then printed what(): "Error: etc.."
		 *
		 * @return A const char of the response of NotRootError
		 */
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "NotRootError: You need to be UID(0)! %s", whatType);
			return retArr;
		}
	private:
		const char *whatType;
	};

	/**
	 * @brief Handles the throwing on the return of a builtin C function
	 *
	 * This is one of the big ports of neo from C to C++, to actually have
	 * throwable errors and not just returned error codes. This is usually called
	 * on the end of every C function call. Yeah it has a lot of arguments, but it's
	 * easier to use this than to write it everytime.
	 * 
	 * @param retType the returned status of the C function
	 * @param pin the pin that's being used
	 * @param low if showing low factor then put it
	 * @param high highest value if available
	 * @param pdd Period or Duty Cycle value (Usually not called within same function)
	 * @param what What system is being used
	 * @param type The error message to be said if any
	 *
	 * @note This can accept any numerical input type
	 */
	template<typename T>	
	inline void Handler(const T retType, int pin = 0, int low = 0, int high = 0, 
						int pdd = 0, const char *what = "", 
									const char *type = "") {
		int theRet = static_cast<int>(retType); //Handle it to an int 
		//(Some return floats, etc... just easier on this end)

		//if(strlen(type) > 0 && strlen(what) > 0) printf("%s: %s\n", what, type); //If what and type aren't blank, print them

		switch(theRet) {
			case NEO_PIN_ERROR:
				throw PinError(pin, low, high);
			case NEO_PERIOD_ERROR:
				throw PeriodError(pdd);
			case NEO_UNUSABLE_ERROR:
			case NEO_UNUSABLE_EXPORT_ERROR:
				throw UnusableError(what, pin);
			case NEO_DUTY_ERROR:
				throw DutyError(pdd, 0, 255); //Default duty low is 0 and high is 255
			case NEO_EXPORT_ERROR:
				throw InitError(what);
			case NEO_DIR_ERROR:
				throw DirError(pdd);
			case NEO_SCALE_ERROR:
			case NEO_READ_ERROR:
				throw ReadWriteError(what, type);
			default: break;
		}
	}

}} //End neo::error

#endif //NEOERROR_H
#endif //__cplusplus
