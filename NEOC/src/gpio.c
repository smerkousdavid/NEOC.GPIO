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
 * @file gpio.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief GPIO controls and pinouts, extremely fast and is used for fake pwm
 *
 * This is the source of the gpio controllers for both the inner and outer banks
 * of the Udoo Neo. Just to be safe make sure the m4 core is disabled. @see neo_disable_m4()
 * 
 * @note Please make sure the m4 core is disabled so that you don't ruin the board
 * @note If you are advanced and still want to use m4, make sure you don't set the same pins to output
 */

#include <neo.h>

#ifndef DOXYGEN_SKIP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <pthread.h>
#include <fcntl.h>

//Array with all of the mapped ports to the correct bank numbers as indexes - 1
const char * const GPIOPORTS[] = {"178", "179", "104", "143", "142", "141", "140", 
			"149", "105", "148", "146", "147", "100", "102",
			"102", "106", "106", "107", "180", "181", "172", 
			"173", "182", "124", "25", "22", "14", "15", "16",
			"17", "18", "19", "20", "21", "203", "202", "177", 
			"176", "175", "174", "119", "124", "127", "116", 
			"7", "6", "5", "4"};

//When attempting to export test all ports as well as document the direction and setting
unsigned char USABLEGPIO[GPIOPORTSL], DIRGPIO[GPIOPORTSL], VALGPIO[GPIOPORTSL];

//P = port pin D = direction pin E = edge L = active low
FILE* gpioP[GPIOPORTSL];
FILE* gpioD[GPIOPORTSL];
FILE* gpioE[GPIOPORTSL];
FILE* gpioA[GPIOPORTSL];

//Double free and initializing error fixed by global flag
unsigned char neo_gpio_freed = 2;

//Set the global exit function
unsigned char neo_exit_set = 2;

//Define the function pointer interrupt to attach to
typedef void (*interruptfunc)(int, int);

struct interrupt_h {
	int pinNum; //Currently handled pin number
	int attached; //If pin is attached to interrupt
	int fd; //File descriptor for watcher
	
	interruptfunc intfunc; //The callback function to handle attachment
};

//Declare alias for struct
typedef struct interrupt_h interrupt_t;

//Thread managing for the gpio interrupts
interrupt_t neo_gpio_interrupts[GPIOPORTSL + 2];

const int buf_r_size = 3; //3 char buffer max (to be generous to the reading)

static gboolean __neo_interrupt_event(GIOChannel *ch, GIOCondition cond, 
				gpointer data) {
	gchar r_buff[buf_r_size]; //Store data buffer
	gsize current_read = 0; //Current byte size read
	GError *err = 0; //Error holder
	
	g_io_channel_seek_position(ch, 0, G_SEEK_SET, 0); //Set data buffer seek to 0
	GIOStatus gs = g_io_channel_read_chars(ch, r_buff, buf_r_size - 1,
										&current_read, &err);
	
	//if((int) gs != 1) { //Check for errors
		int f_data; //Store data from interrupt event
		int cr = sscanf(r_buff, "%d", &f_data); //Load the gchar array into the int
		
		if(cr < 1) { //On data recieve failed
			f_data = NEO_FAIL; //Set failure flag
		}
		
		((interrupt_t *) data)->intfunc(((interrupt_t *) data)->pinNum, f_data);
		//Call the user function with pinNumber and current flag
		
		
	//} else printf("Interrupt error!\n");
	
}

//When the interrupt pin was failed or wasn't assigned by user, print dummy event
void __neo_dummy_int_event(int pin, int data) {
	printf("Dummy Interrupt Event(Pin: %d) -> Data: %d", pin, data);
}


//This will make sure all the interrupts have presets and don't crash after free
void __neo_initialize_interrupts() {
	int ind;
	//Loop through each struct set default params
	for(ind = 0; ind < GPIOPORTSL; ind++) {
		interrupt_t t_temp = neo_gpio_interrupts[ind];
		
		//Set the default values and functions
		t_temp.pinNum = 0;
		t_temp.attached = 0;
		t_temp.intfunc = &__neo_dummy_int_event;
	}
}

//This is the backend thread start that will attach a pin to an interrupt
void *__neo_attach_interrupt(void *arg) {
	interrupt_t pin_fd;
	pin_fd = (*(interrupt_t*)(arg)); //Parse argument as int
	GMainLoop *loop = g_main_loop_new(0,0); //Create new watch loop
	
	GIOChannel *ch = g_io_channel_unix_new(pin_fd.fd); //Set up new channel for file descriptor
	GIOCondition cond = G_IO_PRI; //Only wait for new data to arrive	
	
	//Attach the new loop with the channel on watch
	guint id = g_io_add_watch(ch, cond, __neo_interrupt_event, &pin_fd);
	
	g_main_loop_run(loop); //Start the loop
}

//Thread main loop managers for the pin interrupts
pthread_t gpioINTS_T[GPIOPORTSL + 2];

//Handle freeing all of the functions
void neo_free_all() {
	//Called on exit of program
	printf("FREEING\n");
	neo_gpio_free();
	neo_pwm_free();
	neo_analog_free();
	neo_temp_free();
	neo_accel_free();
	neo_gyro_free();
}


#endif

/**
 * @brief Initializes the GPIO controller
 * 
 * Starts the gpio pins all having the direction of in and  are tested against
 * each other to see if the pin is available. For a fast usable gpio list. This can
 * be called multiple times safely.
 * 
 * @return NEO_OK or NEO_EXPORT_ERROR/NEO_UNUSABLE_ERRROR if some GPIO weren't initialized
 * 
 * @note Don't use the same GPIO pin as PWM although they're mapped differently don't do it
 * @note NEO_UNUSABLE_ERROR might return if it's already in use or PWM is mapped to it
 */
int neo_gpio_init() 
{
	int i, gi, fail;

	
	fail = NEO_OK; //Return code

	//Double check to not run twice
	if(neo_gpio_freed == 2) {
		//Setup cleanup on exit of application
		if(neo_exit_set == 2) {
			atexit(neo_free_all);
			neo_exit_set = 1;
		}
	
		//Set all the ports to usable
		for(gi = 0; gi < GPIOPORTSL; gi++) {USABLEGPIO[gi] = 1;}
	
		//Compile the export sysfs path
		FILE *eFile;
		eFile = fopen(EXPORTPATH, "w");
		
		if(eFile == NULL) fail = NEO_EXPORT_ERROR; //If couldn't export it failed
	
		//Initialize all the gpio ports by looping through available ones
		for(i = 0; i < GPIOPORTSL; i++) { 
	
			//Check if opening of sysfs was a success and attempt to flush the new pin number
			if(eFile != NULL) {
				fprintf(eFile, "%s", GPIOPORTS[i]);
				fflush(eFile);
			}
	
			//Calcuate total new path size to store in buffer
			size_t newPathS = strlen(GPIOPORTS[i]) + gpioL + valueL;
			size_t newDPathS = strlen(GPIOPORTS[i]) + gpioL + directionL;
			size_t newEPathS = strlen(GPIOPORTS[i]) + gpioL + edgeL;
			size_t newAPathS = strlen(GPIOPORTS[i]) + gpioL + activelowL;
	
			//Create new path buffer for temporary storage
			char buff[newPathS];
			char buffD[newDPathS];
			char buffE[newEPathS];
			char buffA[newAPathS];
	
			//Combine the current gpio path to buffer
			sprintf(buff, "%s%s%s", GPIOPATH, GPIOPORTS[i], VALUEPATH);
			sprintf(buffD, "%s%s%s", GPIOPATH, GPIOPORTS[i], DIRECTIONPATH);
			sprintf(buffE, "%s%s%s", GPIOPATH, GPIOPORTS[i], EDGEPATH);
			sprintf(buffA, "%s%s%s", GPIOPATH, GPIOPORTS[i], ACTIVELOWPATH);
	
			//Open the value port pin and direction pins
			gpioP[i] = fopen(buff, "r+");
			gpioD[i] = fopen(buffD, "r+");
			gpioE[i] = fopen(buffE, "r+");
			gpioA[i] = fopen(buffA, "r+");
	
			FILE *port_f = gpioP[i];
			FILE *direction_f = gpioD[i];
	
			//If failed to open the sysfs files make sure to print it's unusable
			if(port_f == NULL || direction_f == NULL) {
				fail = NEO_UNUSABLE_ERROR;
				USABLEGPIO[i] = 0;
			} else {
				fprintf(direction_f, "%s", "in"); //Set all pins to input
				fflush(direction_f); //Flush stream/buffer
			}
			
			DIRGPIO[i] = INPUT; //Set the global direction to 0 (input)
			VALGPIO[i] = LOW; //Set the global pin direction to 0 (low)
			
			FILE *edge = gpioE[i];
			FILE *activelow = gpioA[i];
			
			if(edge != NULL) { //Check that the pin supports interrupts
				fprintf(edge, "%s", "none"); //Set to no interrupt detection
				fflush(edge); //Flush buffer
			}
			
			if(activelow != NULL) { //Check that the pin supports active_low
				fprintf(activelow, "%s", "0"); //Set to default pull down resistor
				fflush(activelow); //Flush buffer
			}
		}
		
		__neo_initialize_interrupts(); //Setup interrupt structs
		
		neo_gpio_freed = 0; //Set initialized flag
	}

	return fail;
}

/**
 * @brief Sets the direction of the pin
 * 
 * Sets the direction of the pin to either two states INPUT OR OUTPUT if it's not either one
 * Of those a NEO_DIR_ERROR will be returned. If teh pin is not a valid pin then it can return either 
 * a NEO_PIN_ERROR (Out of range) or a NEO_UNUSABLE_ERROR (Pin not valid or is in use by another component)
 * 
 * @return NEO_OK/NEO_DIR_ERROR/NEO_PIN_ERROR/NEO_UNUSABLE_ERRROR if some GPIO weren't initialized
 * 
 * @note Don't set to OUTPUT unless you know for sure the m4 (arduino) core isn't using the same pin for OUTPUT
 * @note NEO_UNUSABLE_ERROR might return if the pin is unusable
 */
int neo_gpio_pin_mode(int pin, int direction) {
	//Safety check to see if both arguments are valid
	if(direction < 0 || direction > 1) return NEO_DIR_ERROR;
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;

	if(DIRGPIO[pin] == (unsigned char) INPUT && direction == OUTPUT) { 
		FILE *edge = gpioE[pin]; //Make sure the edge is off to switch to output
		if(edge == NULL || !USABLEGPIO) return NEO_INTERRUPT_ERROR;
		fprintf(edge, "%s", NOEDGE); //Update the edge
		fflush(edge); //Flush the stream
	}

	//Select direction of the pin
	FILE *curD = gpioD[pin];
	if(curD == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;
	fseek(curD, 0, SEEK_SET);
	//Set to INPUT "in" or OUTPUT "out"
	fprintf(curD, "%s", (direction == 0) ? "in" : "out");
	fflush(curD);
	
	DIRGPIO[pin] = (unsigned char) direction;
	
	return NEO_OK; //On success
}

/**
 * @brief Attaches an interrupt to a pin
 * 
 * This will start a new thread that listens for a pin change state based on what you want
 * A state of either "rising" to only detect when the pin changes from 0 to 1. "falling" for the
 * function to only call from 1 to 0 and "both" that happens if it changes either way.
 * 
 * @return NEO_OK/NEO_INTERRUPT_ERROR/NEO_PIN_ERROR/NEO_UNUSABLE_ERRROR if the interrupt failed
 * 
 * @note Don't set to OUTPUT unless you know for sure the m4 (arduino) core isn't using the same pin for OUTPUT
 * @note NEO_UNUSABLE_ERROR might return if the pin is unusable
 */
int neo_gpio_attach_interrupt(int pin, const char * mode, interruptfunc *intfunc) {
	//Safety check to see if both arguments are valid
	if(strcmp(mode, "both") != 0 && strcmp(mode, "rising") != 0 
				&& strcmp(mode, "falling") != 0) { printf("HAHAHAHA\n");  return NEO_INTERRUPT_ERROR; }
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;
	if(intfunc == NULL) { printf("NAH\n"); return NEO_INTERRUPT_ERROR; }

	//If the pin is output, set the pin to input and setup the edge
	if(DIRGPIO[pin] == (unsigned char) OUTPUT) { 
		int ret = neo_gpio_pin_mode(pin, INPUT);
		
		if(ret != NEO_OK) return ret;
	}
	
	FILE *edge = gpioE[pin]; //Make sure the edge is off to switch to output
	if(edge == NULL || !USABLEGPIO) return NEO_INTERRUPT_ERROR;
	fprintf(edge, "%s", mode); //Update the edge
	fflush(edge); //Flush the stream
	
	interrupt_t temp_int = neo_gpio_interrupts[pin];
	temp_int.pinNum = pin;
	temp_int.attached = 1;
	temp_int.intfunc = intfunc;
	temp_int.fd = fileno(gpioP[pin]);
	neo_gpio_interrupts[pin] = temp_int;
	
	pthread_create(&gpioINTS_T[pin], NULL, __neo_attach_interrupt, 
								&neo_gpio_interrupts[pin]);
	
	return NEO_OK; //On success
}

#ifndef DOXYGEN_SKIP

/*
 * This is a hidden method from documentation because we don't want the user
 * to have seg faults or screw up the GPIO because there isn't any safety. The reason this method
 * has been created is for the speed of the FAKE PWM MANAGER
 */
int neo_gpio_digital_write_no_safety(int *pin, int direction) {
	char toPut[2];
	FILE *curP = gpioP[(*pin)];
	
	//Reload seek to cursor set 0 and write current direction
	fseek(curP, 0, SEEK_SET);
	sprintf(toPut, "%d", direction);
	fputc(toPut[0], curP);
	fflush(curP);
}

#endif

/**
 * @brief Writes either LOW or HIGH to gpio pin
 * 
 * This will set the board pin to either a low or high state. Low will be 0 volts and 
 * high will be roughly 3.3v. Since this board works on 3.3v don't use 5v as an input!
 * 
 * @return NEO_OK or NEO_DIR_ERROR/NEO_PIN_ERROR/NEO_UNUSABLE_ERROR if some GPIO write failed
 * 
 * @note You must call neo_gpio_pin_mode(<pin>, OUTPUT); before writing
 * @warning Do not use 5v with these boards! It will ruin the board
 */
int neo_gpio_digital_write(int pin, int direction) {
	//Safety check to see if both arguments are correct
	if(direction < 0 || direction > 1) return NEO_DIR_ERROR;
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;

	if(DIRGPIO[pin] == (unsigned char) INPUT) {
		FILE *active = gpioA[pin]; //Get the pullup resistor
		fprintf(active, "%d", direction); //Set the pullup or pulldown direction
		fflush(active); //Update stream
		VALGPIO[pin] = (unsigned char) direction; //Set the direction currently
		return NEO_OK;					
	}


	FILE *curP = gpioP[pin];

	//Check USABLEGPIO pin
	if(curP == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;

	//Above method to write to the GPIO (Safety check already done)
	neo_gpio_digital_write_no_safety(&pin, direction);

	VALGPIO[pin] = (unsigned char) direction;

	return NEO_OK;
}

/**
 * @brief Reads either LOW or HIGH from gpio pin
 * 
 * This will read the boards current state. If the number is a negative number then the result
 * has failed. Either than that the result will either return a LOW or HIGH value. Same as arduino.
 * 
 * @return NEO_OK or NEO_READ_ERROR/NEO_PIN_ERROR/NEO_UNUSABLE_ERROR if some GPIO read failed
 * 
 * @note You must call neo_gpio_pin_mode(<pin>, INPUT); before reading 
 * @warning Do not use 5v with these boards! It will ruin the board
 */
int neo_gpio_digital_read(int pin) {

	//Safety check for the pin
	if(pin < 0 || pin > GPIOPORTSL) return NEO_PIN_ERROR;

	//You can't read it unless it's INPUT so just return the last known value
	if(DIRGPIO[pin] == (unsigned char) OUTPUT) {
		return VALGPIO[pin];
	}

	FILE *curP = gpioP[pin];

	if(curP == NULL || !USABLEGPIO[pin]) return NEO_UNUSABLE_ERROR;

	int dir;
	fflush(curP); //Update buffer
	fseek(curP, 0, SEEK_SET);
	if(fscanf(gpioP[pin], "%d", &dir) == EOF) return NEO_READ_ERROR; //Read single digit
	return dir;
}

/**
 * @brief Releases the gpio pins from program
 * 
 * This is the exact opposite of neo_gpio_init where you deinitialize the pins
 * Freeing up the pins for another process to use
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERROR if a pin failed to release (most likely will happen)
 */
int neo_gpio_free() 
{
	int fail;
	int i;

	fail = NEO_OK;

	if(neo_gpio_freed == 0) {

		for(i = 0; i < GPIOPORTSL; i++) {
			if(USABLEGPIO[i]) {
				FILE *curP = gpioP[i];
				FILE *curD = gpioD[i];
				FILE *curE = gpioE[i];
				FILE *curA = gpioA[i];
	
				if(curP != NULL) fclose(curP);
				else fail = NEO_UNUSABLE_ERROR;
				if(curD != NULL) fclose(curD);
				else fail = NEO_UNUSABLE_ERROR;
				if(curE != NULL) fclose(curE);
				else fail = NEO_UNUSABLE_ERROR;
				if(curA != NULL) fclose(curA);
				else fail = NEO_UNUSABLE_ERROR;
			}
		}
		neo_gpio_freed = 2;
	}
	
	return fail;
}

/** \page gpioblink Blink
 * \brief How to use the Gpio to Blink an led on pin 13
 *
 * This is how to blink using pin 13 (inner bank same as arduino) in C and C++
 * 
 * If this is your first time I would recommend vim or nano when trying to edit files on the Udoo
 * Vim is obviously better and just straight up amazing, but nano usually suites beginners.
 *
 * Lets create a new file called blink.c (For C++ do blink.cpp)
 *  \code{.sh} nano blink.c \endcode 
 *
 *
 *  In the file try typing this out or copying and pasting it from your browser (Typing it out helps you remember things)<BR>
 * 
 * \section examplec C Example
 *
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
 * <BR>
 * \section examplecpp C++ Example
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
 *      led.setOut(); //Set pin to output mode
 *
 *      for(int ind = 0; ind < 10; ind++)  {
 *         led.on(); //Turn pin on
 *         this_thread::sleep_for(chrono::milliseconds(1000));
 *         led.off(); //Turn pin off
 *         this_thread::sleep_for(chrono::milliseconds(1000));
 *      }
 *     //Auto release since the class releases the pin
 *  }
 * \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc blink.c -I/usr/include -lneo -o blink
 *   chmod 755 blink
 *   ./blink
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ blink.cpp -I/usr/include -lneo -o blink
 * \endcode
 * <BR>
 * \section results Results
 * From the example code to compiling then running the code. You should see pin 13 the <B>ORANGE</B>
 * led on your Udoo NEO blink on and off every one second. This is litteraly the "Hello World!" of embedded devices.
 * Look at that your Udoo is saying hi to you! Wave back... 
 */









/** \page gpioread Reading
 * \brief How to use the Gpio to read from pin 5
 *
 * This is how to digitally read using pin 5 (inner bank same as arduino) in C and C++
 * 
 * Digitally reading a pin is a way of reading the current static voltage of a pin, although imagine it
 * just being in two states. There is HIGH (For the neo is 3.3v) and LOW (0v) of course it's not exactly ever either of them
 * so they are clipped up and down. So anything below 1v will be considered LOW and anything above 2v will be considered HIGH.
 * That's essentially what clipping is (Those aren't the exact real values, just used as an example)
 *
 * Lets create a new file called readgpio.c (For C++ do readgpio.cpp)
 *  \code{.sh} nano readgpio.c \endcode 
 *
 *
 *  In the file try typing this out or copying and pasting it from your browser (Typing it out helps you remember things)<BR>
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
 *    neo_gpio_init(); //Must be called before using any other gpio pin function 
 *
 *    neo_gpio_pin_mode(5, INPUT); //Set pin 5 to input
 *
 *    int gpio_val = neo_gpio_digital_read(5); //Read from pin 5
 *
 *    if(gpio_val != HIGH || gpio_val != LOW) { //Check if the read failed (look at documentation for error codes)
 *       printf("Failed reading from pin 5!\nERROR: %d\n", gpio_val);
 *       return 1;
 *    }
 *
 *    printf("Read from pin 5: %d\n", gpio_val);
 *
 *    neo_gpio_free(); //Should be called to release the pins from the program (Called at end of program)
 *    return 0; 
 * }
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
 *      Gpio readpin(5); //Start pin 5 as gpio pin
 *      readpin.setIn(); //Set as INPUT      
 *
 *      cout << "Read from pin 5: " << readpin.read() << endl; //Read from pin and print result
 *     return 0;
 *  }
 * \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc readgpio.c -I/usr/include -lneo -o readgpio
 *   chmod 755 readgpio
 *   ./readgpio
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ readgpio.cpp -I/usr/include -lneo -o readgpio
 * \endcode
 * <BR>
 * \section results Results
 * After running the program you should get a print in the terminal of "Read from pin 5: <0 or 1>"
 * The pin can have some static and be pulled either direction to make sure you are reading 1 try jumping a wire
 * from 3.3v to pin 5 run it quickly and unplug. Then try attaching gnd to pin 5 and run the program. Does that number change?
 */


//Commented out main code for example use
/*
int main(int argc, char ** argv) {
	printf("INIT\n");
	neo_gpio_init();
	printf("MODE\n");
	neo_gpio_pin_mode(13, OUTPUT);
	printf("WRITE");
	int i;
	sscanf(argv[1], "%d", &i);
	while(1) {
		neo_gpio_digital_write(13, HIGH);
		usleep(50);
		neo_gpio_digital_write(13, LOW);
		usleep(5000);
	}
	//sleep(1);
	//neo_gpio_digital_write(13, LOW);
	//sleep(1)r
	neo_gpio_pin_mode(12, INPUT);
	printf("Mode: %d\n", neo_gpio_digital_read(13));
	neo_gpio_free();
	return 0;
}*/
