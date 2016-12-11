#ifdef __cplusplus

#ifndef NEOERROR_H
#define NEOERROR_H

#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include <stdio.h>

//neo::error::XXXX...
namespace neo { namespace error {
	class PinError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		PinError(int p, int l, int h) : std::runtime_error("PinError"), pin(p), low(l), high(h) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[200]; //Needs to be dynamic since it's out of scope arr
			sprintf(retArr, "PinError: Pin %d is not valid, valid options are %d <-> %d", pin, low, high);
			return retArr;
		}
	private:
		int pin, low, high;
	};

	class PeriodError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		PeriodError(int p) : std::runtime_error("PeriodError"), period(p) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "PeriodError: Period %d is not valid! Check documentation for valid options!", period);
			return retArr;
		}
	private:
		int period;
	};

	class DutyError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		DutyError(int d, int l, int h) : std::runtime_error("DutyError"), duty(d), low(l), high(h) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "DutyError: Duty cycle %d is not valid, valid options are %d <-> %d", duty, low, high);
			return retArr;
		}
	private:
		int duty, low, high;
	};

	class DirError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		DirError(int d) : std::runtime_error("DirError"), direction(d) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[200];
			sprintf(retArr, "DutyError: Direction cycle %d is not valid, valid options are INPUT (0) or OUTPUT (1)", direction);
			return retArr;
		}
	private:
		int direction;
	};

	class UnusableError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		UnusableError(const char * w, int p) : std::runtime_error("UnusableError"), whatType(w), pin(p) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "UnusableError: (%s) -> Pin %d is not usable, make sure it was initialized and it's \
not being used by something else. Best thing to do is check documentation for reason", whatType, pin);
			return retArr;
		}
	private:
		const char *whatType;
		int pin;
	};

	class InitError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		InitError(const char * w) : std::runtime_error("InitError"), whatType(w) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "InitError: %s is not usable, make sure it's not being used by \
something else and you are in root (for safety). Best thing to do is check documentation for reason", whatType);
			return retArr;
		}
	private:
		const char *whatType;
	};

	class ReadWriteError : public std::runtime_error {
	public:
		//Start new runtime error and set local variabls
		ReadWriteError(const char * w, const char *t) : std::runtime_error("ReadWriteError"), 
			whatType(w), typeError(t) {}

		//When called (what()) quit
		virtual const char * what() const throw() {
			char * retArr = new char[350];
			sprintf(retArr, "InitError: (%s) failed to %s. It's still usable just something failed \
to change pin state", whatType, typeError);
			return retArr;
		}
	private:
		const char *whatType, *typeError;
	};

	//Error handler to handle inline the throw
	template<typename T>	
	inline void Handler(const T retType, int pin = 0, int low = 0, int high = 0, int pdd = 0, 
						const char *what = "", const char *type = "") {
		int theRet = static_cast<int>(retType); //Handle it to an int (Some return floats, etc... just easier on this end)

		switch(theRet) {
			case NEO_PIN_ERROR:
				throw PinError(pin, low, high);
			case NEO_PERIOD_ERROR:
				throw PeriodError(pdd);
			case NEO_UNUSABLE_ERROR:
			case NEO_UNUSABLE_EXPORT_ERROR:
				throw UnusableError(what, pin);
			case NEO_EXPORT_ERROR:
				throw InitError(what);
			case NEO_READ_ERROR:
				throw ReadWriteError(what, type);
			default: break;
		}
	}

}} //End neo::error

#endif //NEOERROR_H
#endif //__cplusplus
