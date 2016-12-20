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
 * @file temp.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief Controls the temperature brick module
 *
 * This is the source to read from the temperature brick module
 * 
 * @note You can not use the i2c-2 line and the brick sensors at the same time
 */
#include <neo.h>

#ifndef DOXYGEN_SKIP

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

//The controller reader to get the milli celc from
FILE *neo_temp_controllerr;

//Double free and init failure flag fix
unsigned char neo_temp_freed = 2;

#endif

/**
 * @brief Initializes the temperature controller
 * 
 * Starts the driver for the temperature brick module to be able to read data
 * from it. Extremely simple use and conversion functions.
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERRROR if it failed to load the temperature driver or find the brick
 * 
 */
int neo_temp_init() {
	if(neo_temp_freed == 2) {
		//Setup cleanup on exit of application
		if(neo_exit_set == 2) {
			atexit(neo_free_all);
			neo_exit_set = 1;
		}
		
		//The temperature brick module requires root permission
		neo_check_root("The temperature module requires root!");
		//Restart the module
		system("rmmod lm75"); //Unload the driver
		system("modprobe lm75"); //Reload the i2c brick driver
	
		//Open the sysfs stream to initialize the temperature path
		FILE* creator;
		creator = fopen(TEMPPATH, "w");
	
		if(creator != NULL) {
			fseek(creator, 0, SEEK_SET); //Set the seek of the file to be 0
			fprintf(creator, "%s", TEMPINIT); //Set the export temp init
			fflush(creator); //Flush the buffer
			fclose(creator); //Close the stream
		}
	
		//Set the temperature reading module stream
		neo_temp_controllerr = fopen(TEMPREAD, "r");
		
		//Check to see if the stream is valid
		if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;
		neo_temp_freed = 0; //Set the global freed flag to initialized
	}
	return NEO_OK;
}

/**
 * @brief Reads the current temperature from the brick module
 * 
 * Read the current temperature from the brick snapin sensors. The raw return
 * of this function is in milli celcius.
 * 
 * @return NEO_OK or /NEO_READ_ERROR/NEO_UNUSABLE_ERRROR if it failed to read from the brick
 * 
 */
int neo_temp_read() {
	//Double check to see if the controller is still usable
	if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;

	int milliCelc = 0; //Get store the milliCelc in a var

	fflush(neo_temp_controllerr); //Flush the buffer to sync the file
	fseek(neo_temp_controllerr, 0, SEEK_SET); //Set the seek position to 0 to reread
	if(fscanf(neo_temp_controllerr, "%d", &milliCelc) == EOF) 
		return NEO_READ_ERROR; //Read the current milliCelc and check for EOF

	return milliCelc; //return the positive milliCelc
}

/**
 * @brief Frees the controller from the current program
 * 
 * This will release the temperature brick controller from the program
 * 
 * @return NEO_OK or NEO_UNUSABLE_ERRROR if it failed to release the brick
 * 
 */
int neo_temp_free() {
	//Set the global flag to device is released
	if(neo_temp_freed == 0) {
		//Double check the controller
		if(neo_temp_controllerr == NULL) return NEO_UNUSABLE_ERROR;
		fclose(neo_temp_controllerr); //Release the controller
		neo_temp_freed = 2; //Set flag to freed
	}
	return NEO_OK;
}

/** \page tempbrick Temperature
 * \brief How to use the temperature snapin brick and read from it
 *
 * This is how to load the temperature brick module and read from it
 *
 * Lets create a new file called temp.c (For C++ do temp.cpp)
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
 *    neo_temp_init(); //Must be called before reading from the brick
 *
 *    //Loop 10 times
 *    int i;    
 *
 *    for(i = 0; i < 10; i++) {
 *       int milliCelc = neo_temp_read(); //Read the current temperature
 *       printf("Temp (mC): %d\n", milliCelc); //Print the milli celcius
 *       sleep(1); //Sleep for one second
 *    }
 *
 *    neo_temp_free(); //Good practice to call this before exit
 *    return 0;
 *  }
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
 *  //Main function
 *  int main() {
 *      TempBrick::init(); //Initialize the brick for reading
 *
 *      //Loop ten times
 *      for(int ind = 0; ind < 10; ind++)  {
 *         int milliCelc = TempBrick::read(); //Read from the brick
 *         cout << "Temp (mC): " << milliCelc << endl;
 *         //Print the milli celcius      
 *
 *         //Sleep for one second   
 *         this_thread::sleep_for(chrono::milliseconds(1000));
 *      }
 * 
 *      TempBrick::free(); //Release the i2c brick
 *
 *      return 0;
 *  }
 * \endcode
 * <BR>
 * \subsection compiling Compiling and Running
 * This is the example to compile and run the C version of neo
 * \code{.sh}
 *   gcc temp.c -lneo -o temp
 *   chmod 755 temp
 *   ./temp
 * \endcode
 * <BR>
 * The C++ version to compiling is nearly the same just replace gcc with g++
 * \code{.sh}
 *   g++ temp.cpp -lneo -o temp
 * \endcode
 * <BR>
 * \section results Results
 * You should see ten newlined prints of the temperature results. Hopefully
 * it doesn't change too much and the brick is working.
 */


/*
int main() {
	neo_temp_init();
	printf("TEMP: %d\n", neo_temp_read());
	neo_temp_free();
}*/
