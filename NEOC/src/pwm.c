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
 * @file pwm.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief The file that controls the real and fake pwm controllers
 *
 * This source file is the core of the FAKE PWM MANAGER which controls
 * A threaded pwm manager on ANY gpio pin available on the board as well as
 * A real PWM access to the available pwm pins.
 * 
 * @note Please use device tree editor to view what pwm pins are available
 */

#include <neo.h>

#ifndef DOXYGEN_SKIP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

//All the pin numbers for the pwm pins and the flags for if they're usable
const char * const PWMPORTS[] = {"0", "1", "2", "3", "4", "5", "6"};
unsigned char USABLEPWM[] = {0, 0, 0, 0, 0, 0, 0};

//A list of the fake pwm allowed
//Look at MAXFAKEPWM in neo.h to see the max allowed PWM
//Size safety of 2 for each for the loops
int FAKEPWMLIST[MAXFAKEPWM + 2][2];

//FILE pointers to sysfs P = period D = duty and E = enable/disable
FILE* pwmP[PWMPORTSL + 2];
FILE* pwmD[PWMPORTSL + 2];
FILE* pwmE[PWMPORTSL + 2];

//Defaults for period and duty cycles currently set to 49Khz and 50%
unsigned int neo_pwm_period = 20408;
unsigned int neo_pwm_duty = 50;

//Couting of the fake pwm list
int neo_pwm_counting = 0;


//Double free or nothing error fix and doesn't double initialize
unsigned char neo_pwm_freed = 2;

//Thread managers that manage the pwm frequency and GPIO switching
pthread_t fakePWMT[MAXFAKEPWM + 2];


#endif

//The struct that will be muxed between the main thread and FAKEPWMMANAGER
//To handle the current duty cycle and low high periods
struct params {
	pthread_mutex_t mutex;
	pthread_cond_t done;
	int pin, high, low, updatePeriod;
};

//Create the new params struct to not confuse locals
typedef struct params params_t;

#ifndef DOXYGEN_SKIP

//Creeate the same structured list to easily access the managers (The index
//Doesn't matter)
params_t threadProps[MAXFAKEPWM + 2];

#endif

/**
 * @brief Initializes the REAL pwm pins on the Udoo
 * 
 * Starts the internal PWM controllers. This is recommended over FAKEPWM since
 * It is actually pwm that has a real controller. Although this is not recommended
 * Since it can conflict with the GPIO pin that's also being used
 * 
 * @return NEO_OK or NEO_EXPORT_ERROR if some PWM weren't initialized
 * 
 * @note Do not use the same GPIO as output or input! It will override this
 * @note NEO_UNUSABLE_ERROR might be returned if that pin doesn't support PWM or it's not mapped using device tree editor 
 */
int neo_pwm_init()
{
	int i;
	int fail;

	fail = NEO_OK; //Return code

	//Don't initialize twice
	if(neo_pwm_freed == 2) {
		FILE *eFile;
		//Export the GPIO pin to be used with sysfs
		eFile = fopen(PWMEXPORTPATH, "w");

		//Configure all possible PWM pins
		for(i = 0; i < PWMPORTSL; i++) {
			
			//If the export completed export that pin number
			if(eFile != NULL) {
				fprintf(eFile, "%s", PWMPORTS[i]);
				fflush(eFile);
				USABLEPWM[i] = 1;
			} else fail = NEO_EXPORT_ERROR;
	
	
			//New char path size fixes, C is sometimes repetitive...
			//The sizes of the paths can be found in neo.h
			size_t pwmSize = strlen(PWMPORTS[i]) + pwmL + 5;
			size_t newPathS = pwmSize + pwmPeriod;
			size_t newDPathS = pwmSize + pwmDuty;
			size_t newEPathS = pwmSize + pwmEnable;
	
			//Create the new char arrays for the paths to be stored in
			char buffP[newPathS];
			char buffD[newDPathS];
			char buffE[newEPathS];
	
			//Combine the paths together into one per PWM pin
			//And copy them into the above buffer to be later opened and checked
			sprintf(buffP, "%s%s%s", PWMPATH, PWMPORTS[i], PWMPERIOD);
			sprintf(buffD, "%s%s%s", PWMPATH, PWMPORTS[i], PWMDUTY);
			sprintf(buffE, "%s%s%s", PWMPATH, PWMPORTS[i], PWMENABLE);
	
			//Load the period into the program
			pwmP[i] = fopen(buffP, "w");
	
			//If the pwm failed to load, then still the PWM pin is unusable
			//Set error flag to NEO_UNUSABLE_EXPORT_ERROR (meaning fully disfunctional)
			if(pwmP[i] != NULL) {
				fprintf(pwmP[i], "%d", neo_pwm_period);
				fflush(pwmP[i]);
			} else { 
				fail = (fail == NEO_EXPORT_ERROR || fail == NEO_UNUSABLE_EXPORT_ERROR) ? 
						NEO_UNUSABLE_EXPORT_ERROR : NEO_UNUSABLE_ERROR; 
				USABLEPWM[i] = 0;
			}
	
			//Try the same for Duty cycle and the enable(r) of PWM
			pwmD[i] = fopen(buffD, "w");
			pwmE[i] = fopen(buffE, "w");
	
			if(pwmD[i] == NULL || pwmE[i] == NULL) {
				fail = (fail == NEO_EXPORT_ERROR || fail == NEO_UNUSABLE_EXPORT_ERROR) ? 
						NEO_UNUSABLE_EXPORT_ERROR : NEO_UNUSABLE_ERROR; 
				USABLEPWM[i] = 0;
				continue;
			}
			
			neo_pwm_set_period(i, neo_pwm_period); //Set the default 49KHz for all the pins
		}
		//Set the global flag to see if PWM is initialized
		neo_pwm_freed = 0;	
	}
	return fail;
}

//Don't add this to the documentation page
#ifndef DOXYGEN_SKIP

/*
 * Backend function to control the synchronization of the thread muxing for
 * the params_t struct. All args are pointers since it's called within the 
 * FAKEPWMMANAGER thread
 */
void neo_sync_pwm(void *arg, int *pin, int *high, int *low,
		int *updatePeriod) {
	//Lock onto main thread to gain the struct via the params_t struct mutex
	pthread_mutex_lock(&(*(params_t*)(arg)).mutex);

	//Loaded params_t struct from teh void pointer in arg
	params_t sets = (*(params_t*)(arg));

	*pin = sets.pin;
	*high = sets.high;
	*low = sets.low;
	*updatePeriod = sets.updatePeriod;

	//Unlock the struct from the main thread
	pthread_mutex_unlock(&(*(params_t*)(arg)).mutex);
}

/*
 * This is the thread that will be created everytime the user inits a new
 * FAKEPWM_WRITE on a new GPIO pin that doesn't exists. From there the write method
 * Will check all the params struct to see if the pin already exists and attempt to update that struct
 * Of which the thread manager should update within 17% of the millis period time (On 49Hz) That's 200 millis (A little slow)
 */
void *pwmManager(void *arg){
	int pin, high, low; //Pin settings for params sync
	
	//counter for update period and global setting for params sync
	//BEFORE: unsigned long long (unecessary)
	int counter, updatePeriod;

	//Initial sync of settings since usuably write has completed by now
	neo_sync_pwm(arg, &pin, &high, &low, &updatePeriod);
	
	//No other debug method but printing
	//This should be a flag, for now it works
	//@TODO change debug of FAKEPWMMANAGER to a flag
	printf("FAKE PWM PORT: %d\n", pin);

	//Star the pin in low and set the pinMode to output for safety
	int retM = neo_gpio_pin_mode(pin, OUTPUT);
	int remR = neo_gpio_digital_write(pin, 0);


	/*
	If either the pin_mode or digital_write fail on us... Do not continue!
	It's crucial not to continue because it either means the GPIO are already
	In use or that the pin failed to initialize. There is no safety pased this point
	Better stop the FAKEPWMMANAGER now and just print an error, so the user can see it failed
	*/
	if(retM != NEO_OK) {
		printf("FAKE PWM FAILURE, COULDN'T EXPORT PIN!\n\n");
		return NULL;
	}

	if(remR != NEO_OK) {
		printf("FAKE PWM FAILURE, COULDN'T LOWER PIN");
		return NULL;
	}
	printf("STARTING FAKE PWM MANAGER ON PIN: %d\n", pin);
	
	//Set the current pin update counter
	counter = 0;
	//Loop forever until the thread manager is killed via a sleep interrupt
	while(1) {
		//The update period is calculated on fake_pwm_write @see neo_fake_pwm_write_period for more details
		if(counter > updatePeriod) {
			neo_sync_pwm(arg, &pin, &high, &low, &updatePeriod);
			counter = 0;
		}
		counter++;
		//The no safety option makes the write faster since it's not checking
		neo_gpio_digital_write_no_safety(&pin, HIGH); //Pull the duty cycle high
		if(usleep(high) != 0) break; //Wait for the high part of duty cycle to finish
		//For anything just writing to the pin
		//If error happens then a segmentation fault must have occured on writing
		//To a NULL pin
		neo_gpio_digital_write_no_safety(&pin, LOW); //Pull the duty cycle to low
		if(usleep(low) != 0) break; //Make sure low finished (usleep is slightly off but doesn't eat 100% CPU (not a CPU cycle counter)
	}
}

#endif

/**
 * @brief Initializes the FAKE pwm, p.s this is the same as neo_gpio_init
 * 
 * Initializes the gpio pins for the pwm manager, there is no worry calling this twice
 * It's just unnecessary if you're using neo_gpio_init before this method
 * 
 * @return Look at neo_gpio_init for return types. NEO_OK when successful
 */
int neo_fake_pwm_init() {
	return neo_gpio_init(); //Initialize the gpio pins @see neo_gpio_init in gpio.c
}

/**
 * @brief Main write method for fake_pwm
 * 
 * Thise will create a new FAKEPWMMANAGER thread on the selected bank GPIO pin
 * The manager will only be created if there isn't one already on that pin. So don't worry about
 * Calling this method multiple times. Actually do it when you want to update the duty cycle.
 * @see neo_fake_pwm_write 
 * If you want to see the regular duty cycle update. The second argument is period.
 * 
 * @param gpioPin The gpio bank pin to set or update the fake pwm manager on
 * @param period The nano second update time for the period
 * @param duty The duty cycle percentage between 0 and 255 (Like arduino)
 * 
 * @return NEO_OK/NEO_DUTY_ERROR or NEO_PIN_ERROR if the params are wrong
 * 
 * @note The period argument is in nano second update time for period so 1000000000 would be 1Hz (Aka 1 loop per second)
 */
int neo_fake_pwm_write_period(int gpioPin, int period, int duty) {
	int i;
	char passed;

	//Check to see if either the pin or the duty cycle are off
	if(gpioPin < 0 || gpioPin > GPIOPORTSL) return NEO_PIN_ERROR;
	if(duty < 0 || duty > 255) return NEO_DUTY_ERROR;

	//Remap the allowed duty cycle between 0 and 255 to the max of period
	//This is for the higher duty cycle sleep time
	int remap = (int) neo_re_map((float)duty, 0.0f, 255.0f, 0.0f,(float)period);
	//Set the lower duty cycle sleep time to the exact opposite of requested high
	int lower = (period - remap);
	//Set the update period of the pin to a static percentage for the same time everytime usually 200 millis
	int upPeriod = (int)(PWMMUTEXRATE * ((float)period)); 

	passed = 1;
	//Check to see if there is already a thread manager on the pin
	for(i = 0; i < MAXFAKEPWM; i++) {
		//Check to see if the sysfs isn't NULL
		if(FAKEPWMLIST[i] != NULL) {
			//Check the params_t struct LIST checker to see if the pin is already mapped
			if(FAKEPWMLIST[i][0] == gpioPin) {
				//The thread manager has been found
				passed = 0;					

				//Pull the threadManagerProperty from the FAKEPWMLIST
				params_t paramL = threadProps[i]; 
			

				//pthread_mutex_lock(&paramL.mutex);

				//Update the props to be later locked by the manager thread
				paramL.pin = gpioPin;
				paramL.high = remap;
				paramL.low = lower;
				paramL.updatePeriod = upPeriod; //20 Cycles

				//Double check and set the LIST to the current pin
				FAKEPWMLIST[i][0] = gpioPin;
				FAKEPWMLIST[i][1] = i;

				//Set the update paramProp pointer to be locked by thread manager on pthread
				threadProps[i] = paramL;

				//pthread_cond_wait (&paramL.done, &paramL.mutex);
				//pthread_mutex_unlock(&paramL.mutex);
			}
		}
	}

	//If the FAKEPWMLIST couldn't find a pwm manager create a new one mapped with a params_t struct
	if(passed) {
		int curI = neo_pwm_counting;
		if(curI > MAXFAKEPWM) return NEO_EXPORT_ERROR;
		
		//Create the new params_t and set the parameters (self explanatory)
		params_t initP;
		initP.pin = gpioPin;
		initP.high = remap;
		initP.low = lower;
		initP.updatePeriod = upPeriod;

		//Update the global array
		threadProps[curI] = initP;	

		//Start the mutex locker on the pthread with the params_t mutex locker
		pthread_mutex_init(&threadProps[curI].mutex , NULL);
		//pthread_cond_init(&threadProps[curI].done, NULL);
		//pthread_mutex_lock(&threadProps[curI].mutex);

		//Create the new thread with the pointer to the pointing array for the params_t
		pthread_create(&fakePWMT[curI], NULL, pwmManager, &threadProps[curI]);
		//pthread_cond_wait (&threadProps[curI].done, &threadProps[curI].mutex);
		
		//Update the fake PWM list to the currently selected GPIO to update the thread manager checking
		FAKEPWMLIST[curI][0] = gpioPin;
		FAKEPWMLIST[curI][1] = curI;	
		neo_pwm_counting++; //Just to set the later indexes and check if the current count is above max
	}
	return NEO_OK; //Return NEO_OK on completion of write
}

/**
 * @brief Duty write method for fake_pwm
 * 
 * This will just write the update duty cycle for the pwm_pin, this will not update the period
 * Unless the global variable is also updated, please make sure that all pins work on same frequency
 * For simplicity reasons. @see neo_fake_pwm_write_period to update the period
 * 
 * @param gpioPin The gpio bank pin to set or update the fake pwm manager on
 * @param duty The duty cycle percentage between 0 and 255 (Like arduino)
 * 
 * @return NEO_OK/NEO_DUTY_ERROR or NEO_PIN_ERROR if the params are wrong
 */
int neo_fake_pwm_write(int gpioPin, int duty) {
	return neo_fake_pwm_write_period(gpioPin, neo_pwm_period, duty); //Run above method with globally set period
}

/**
 * @brief Main write method for real pwm
 * 
 * For the real pwm you don't set the period within the write method but rather than
 * You will just set the duty between 0 (min) and 255 (max) 
 * 
 * 
 * @param pin The gpio bank pin to set or update the fake pwm manager on
 * @param duty The duty cycle percentage between 0 and 255 (Like arduino)
 * 
 * @return NEO_OK/NEO_DUTY_ERROR/NEO_PIN_ERROR or NEO_UNUSABLE_ERROR if the params are wrong
 * @note NEO_UNUSABLE_ERROR might be returned if that pin doesn't support PWM or it's not mapped using device tree editor 
 */
int neo_pwm_write(int pin, int duty) {
	if(pin < 0 || pin > PWMPORTSL) return NEO_PIN_ERROR;
	if(duty < 0 || duty > 255) return NEO_DUTY_ERROR; 

	FILE *curD = pwmD[pin];
	FILE *curE = pwmE[pin];

	if(curD == NULL || curE == NULL || !USABLEPWM[pin]) return NEO_UNUSABLE_ERROR;

	if(duty == 0) {
		fprintf(curE, "%d", 0); //Disable pwm
		fflush(curE);
	} else {
		int remap = (int) neo_re_map((float)duty, 0.0f, 255.0f, 0.0f,(float)neo_pwm_period); 
		fprintf(curE, "%d", 1);
		fflush(curE);
		
		fprintf(curD, "%d", remap);
		fflush(curD);
	}
	return NEO_OK;
}


/**
 * @brief Sets period for all real pwm pins
 * 
 * Updates the period for all the real pwm pins in one function of course you could do it manually
 * With a for loop. This is just for those lazy people who don't want to do that
 * 
 * @param period The nanosecond update period per loop period
 * 
 * @return NEO_OK/NEO_PERIOD_ERROR/NEO_PIN_ERROR or NEO_UNUSABLE_ERROR if the params are wrong
 * @note The period argument is in nano second update time for period so 1000000000 would be 1Hz (Aka 1 loop per second)
 */
int neo_pwm_set_period_all(int period) {
	int i;
	int fail;

	fail = NEO_OK;
	//Loop through all pins
	for(i = 0; i < PWMPORTSL; i++) {
		if(USABLEPWM[i] == 1) {
			//Set the individual pin (@see neo_pwm_set_period)
			int errCode = neo_pwm_set_period(i, period);
			//Check if there is any error code and set the fail to that
			if(errCode != NEO_OK) fail = errCode;
		}		
	}

	return fail;
}

/**
 * @brief Sets period for a single pwm pin
 * 
 * @param pin The pwm pin to set the current
 * @param period The nanosecond update period per loop period
 * 
 * @return NEO_OK/NEO_PERIOD_ERROR/NEO_PIN_ERROR or NEO_UNUSABLE_ERROR if the params are wrong
 * @note The period argument is in nano second update time for period so 1000000000 would be 1Hz (Aka 1 loop per second)
 */
int neo_pwm_set_period(int pin, int period) {
	//Safety check for the pin and the period before continuing
	if(pin < 0 || pin > PWMPORTSL) return NEO_PIN_ERROR;
	if(period < 0 || period > 1000000000) return NEO_PERIOD_ERROR; 


	//Open sysfs period and write the updated period
	FILE *curP = pwmP[pin];
	
	if(curP == NULL || !USABLEPWM[pin]) return NEO_UNUSABLE_ERROR;

	neo_pwm_period = period;
	fprintf(curP, "%d", period);
	fflush(curP);
	return NEO_OK;	
}

/**
 * @brief Main function to release the real pwm
 * 
 * This is the exact opposite of neo_pwm_init where you deinitialize the pins
 * Freeing up the pins for another process to use
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERROR if some pins failed to release (most likely will happen)
 */
int neo_pwm_free()
{
	int fail;
	int i;

	fail = NEO_OK;
	
	if(neo_pwm_freed == 0) {
		//Loop through each pinSet
		for(i = 0; i < PWMPORTSL; i++) {
			if(USABLEPWM[i]) {
				FILE *curP = pwmP[i];
				FILE *curD = pwmD[i];
				FILE *curE = pwmE[i];
	
		
				//Close all three sysfs files
				//The Period, Duty and the Enable
				if(curP != NULL) fclose(curP);
				else fail = NEO_UNUSABLE_ERROR;
	
				if(curD != NULL) fclose(curD);
				else fail = NEO_UNUSABLE_ERROR;
	
				if(curE != NULL) fclose(curE);
				else fail = NEO_UNUSABLE_ERROR;
			}
		}
		neo_pwm_freed = 2;
	}
	return fail;
}


/** \page examplepwm Real
 * \breif How to use the Real PWM controller to set a duty cycle
 *
 * Please look at the pinout to find the PWM number on the bank (The pin isn't the same as GPIO)
 * If you haven't yet make sure you go to the device tree editor and pinmux a pin to support PWM
 * You must do this or you will recieve a NEO_UNUSABLE_ERROR when trying to write to that pin!
 * The real PWM mapping to the pinmuxing is really weird and it's why the functions aren't mapped directly
 * to the actual gpio pin. Sorry but you have to mess around with the pins before getting it to work. For now
 * just try mapping PWM 5 on the device tree editor to GPIO bank pin 30 and trying the below sample code with PWM 0
 * If 0 shoots a NEO_UNUSABLE_ERROR. Try 1... and so forth.<BR><BR>
 * Device Tree Tutorial: https://www.udoo.org/docs-neo/Cookbook_Linux/Device_Tree_Editor.html
 *
 * \warning You must pinmux the proper PWM pin before continuing
 *
 * Lets create a new file called pwmwrite.c (For C++ do pwmwrite.cpp)
 *  \code{.sh} nano pwmwrite.c \endcode 
 *
 * 
 * \section examplec C Example
 *
 *  \code{.c}
 *  #include <neo.h>
 *  #include <stdio.h>
 * 
 *  //Main funciton
 *  int main() {
 *    neo_pwm_init(); //Must be called before using any other gpio pin function 
 *
 *    //neo_pwm_set_period(0, 20408); //OPTIONAL: Set a custom period time 20408 = ~49KHz
 *    //neo_pwm_set_period_all(20408); //OPTIONAL: Set for all the pins
 *
 *    int ret = neo_pwm_write(0, 127); //Set pwm pin 0 (pwm 5) (IF YOU MAP PWM 5 to 
 *    //GPIO pin 30 on device tree then pwm0 is pwm 5)  50% duty cycle (it's 0 to 255)
 *
 *    if(ret != NEO_OK) {
 *       printf("Failed writing to PWM!\nERROR: %d", ret);
 *    }
 *
 *    //This will still run even when the program is finished... Just remember that
 *
 *    neo_pwm_free(); //Should be called to release the pins from the program (Called at end of program)
 *    return 0;
 *  }
 *  \endcode
 * <BR>
 * \section examplecpp C++ Example
 * \code{.cpp}
 *  #include <neo.h>
 *  #include <iostream>
 * 
 *  using namespace neo; 
 *  using namespace std;
 *
 *  //Main function
 *  int main() {
 *      PWM pwm(0); //Start on PWM pin 0 (pwm 5) (NOT THE SAME AS GPIO MAPPING)      
 *      //pwm.setPeriod(20408); //Example set period
 *      pwm.write(127);
 *  }
 * \endcode
 * Warning The mapping of Real PWM is not the same as Gpio. Nor is it linear to the PWM mapping! (Read above)
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc pwmwrite.c -I/usr/include -lneo -o pwmwrite
 *   chmod 755 pwmwrite
 *   ./pwmwrite
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ pwmwrite.cpp -I/usr/include -lneo -o pwmwrite
 * \endcode
 * <BR>
 * \section results Results
 * Try hooking up an LED to that PWM pin and setting the PWM values at different points to see if the lighting changes
 * If nothing changes make sure that pin supports REAL PWM, isn't failing to initialize it and it's been pinmuxed properly
 * In the device tree editor
 */









/** \page examplefakepwm FakePWM
 * \breif How to use PWM on any GPIO pin easily
 *
 * This is a tutorial and example of how to use Fake PWM on any Gpio pin 
 * 
 * Unlike the Real PWM this isn't as precise since it uses the processor clock and not a custom resolution clock
 * This also is down to the speed of sysfs. I was able to do up to 100KHz (About half the period time of an arduino(p.s. that's good))
 * On about 30 pins, before the the led seemed to jitter at a visible amount. So I would recommend to leave it at 49KHz if using FakePWM
 * on a lot of the pins. This is a lot easier to use than the Real PWM because it has the same mapping as the gpio since it uses the same
 * methods in the backend. Try the example below for pin 13 which is the led.
 *
 * Lets create a new file called fakefade.c (For C++ do fakefade.cpp)
 *  \code{.sh} nano fakefade.c \endcode 
 * 
 * \section examplec C Example
 *
 *  \code{.c}
 *  #include <neo.h>
 *  #include <unistd.h>
 *  #include <stdio.h>
 * 
 *  //Main funciton
 *  int main() {
 *    neo_fake_pwm_init(); //Exactly the same as neo_gpio_init()
 *
 *    printf("Starting the fade\n");
 *
 *    while(1) {
 *       int i;
 *       //Fade up to 255
 *       for(i = 0; i < 255; i++) {
 *          neo_fake_pwm_write(13, i); //Write up pwm value
 *          usleep(1000*10); //Wait 10millis
 *       }
 *       int d;
 *       
 *       //Fade down back to 0
 *       for(d = 255; d > 0; d--) {
 *          neo_fake_pwm_write(13, d); //Write down pwm value
 *          usleep(1000*10); //Wait 10millis
 *       }
 *          usleep(1000*400); //Stay low 400 more millis
 *    }
 *
 *    //neo_gpio_free(); <- This should be called on end of program 
 *    return 0; 
 * }
 *  \endcode
 * <BR>
 * \section examplecpp C++ Example
 * \code{.cpp}
 *  #include <neo.h>
 *  #include <chrono>
 *  #include <thread>
 *  #include <iostream>
 * 
 *  using namespace neo; 
 *  using namespace std;
 *
 *  int main() {
 *    FakePWM fpwm(13); //This will also init a static version of gpio
 *
 *    cout << "Starting the fade" << endl;
 *
 *    while(1) {
 *       //Fade up to 255
 *       for(int i = 0; i < 255; i++) {
 *          fpwm.write(i); //Write up pwm value
 *          this_thread::sleep_for(chrono::milliseconds(10));
 *       }
 *       
 *       //Fade down back to 0
 *       for(int d = 255; d > 0; d--) {
 *          fpwm.write(d); //Write down pwm value
 *          this_thread::sleep_for(chrono::milliseconds(10));
 *       }
 *          this_thread::sleep_for(chrono::milliseconds(400));
 *    }
 *    return 0; 
 * }
 *  \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc fakefade.c -I/usr/include -lneo -o fakefade
 *   chmod 755 fakefade
 *   ./fakefade
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ fakefade.cpp -I/usr/include -lneo -o fakefade
 * \endcode
 * <BR>
 * \section results Results
 * Did you see pin 13 fading? Well if it worked without printing any errors you got the FakePWM to work
 * now you can run PWM on any gpio pin available on the Udoo extremely easy. I don't know why I'm telling you this
 * if you are on C and C++ and probably already know the expected results of the program.
 */



//Commented out test main function
//This is an example of how to use fake_pwm
/*int main() {
	neo_gpio_init();
	neo_fake_pwm_write(12, 127);
	neo_fake_pwm_write(11, 30);
	neo_fake_pwm_write(2, 50);
	neo_fake_pwm_write(3, 100);
	neo_fake_pwm_write(4, 100);
	neo_fake_pwm_write(5, 100);
	int a;
	for(a = 16; a < 37; a++) neo_fake_pwm_write(a, 200);	

	while(1) {
		int i;
		for(i = 0; i < 255; i++) {
			neo_fake_pwm_write(13, i);
			usleep(1000*10);
		}
		int d;
		for(d = 255; d > 0; d--) {
			neo_fake_pwm_write(13, d);
			usleep(1000*10);
		}
		usleep(1000*400);
	}
	neo_gpio_free();
	return 0;
}*/
