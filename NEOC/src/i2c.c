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
 * @file i2c.c
 * @author David Smerkous
 * @date 11/28/2016
 * @brief The i2c source file to control all the i2c lines
 *
 * @details This has the functions to control the i2c lines on i2c1/2 and 4. This has all
 * the basics functions that Arduino has. When using i2c2, it will disable the i2c drivers
 * for the temperature and humidity bricks that use the same line.
 */
 
#include <neo.h>

#ifndef DOXYGEN_SKIP

#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


int neo_i2c_fds[I2CCOUNT + 2]; //The file descriptors for the i2c lines

//Double free or nothing error fix and doesn't double initialize
unsigned char neo_i2c_freed[I2CCOUNT + 2];
unsigned char neo_i2c_global_freed = 2; //Setup the arrays when starting over

#endif



/**
 * @brief Initializes an i2c adapter
 * 
 * This can safely be called multiple times, but we don't see the reason why.
 * The init function will attach the program to the i2c kernel driver aka i2c-dev.h
 * This is a wrapper for that to simplify all of the i2c functionality. Just remember the pinout is different from the system type printed 
 *
 * @param adapter The physical hardware i2c adapter number
 * @return NEO_OK/NEO_I2C_ADAPTER_ERROR/NEO_I2C_INIT_ERROR when it failed to init
 * 
 * @note On the I2C-2 line, the brick drivers will be disabled (So you can't use the temp and humidity bricks)
 * @warning I2C-3 is practically not usable because it's on a 1.8v logic line
 * @warning I2C-1 is used up a lot by the power manager, LVDS and JTAG (Check Udoo documentation to see what device addresses are available)
 * @warning I2C-4 addresses 0x1E and 0x20 are used up by the accel/magno/gyro devices
 */
int neo_i2c_init(int adapter) {
	neo_check_root("The i2c control on any adapter requires root!");
	adapter -= I2CSHIFT; //Move the adapter up for real board adapters 1 = 2
	
	//Double check to see if the i2c adapter is valid
	if(adapter < 0 || adapter > I2CCOUNT) return NEO_I2C_ADAPTER_ERROR;

	int i;
	
	//Disable all drivers and print warning that handle i2c on the selected adapter
	switch(adapter) {
		case 0: //The power manager, LVDS connector and JTAG debugger (DANGEROUS)
			printf("WARNING: I2C-1 attempted to be accessed, if not used \
properly this can screw up your system!\n");
			break;
		case 1: //The builtin brick module (Temperature and Humidity)
			system("sudo rmmod mpl3115"); //Disable the barometer driver
			system("sudo rmmod lm75"); //Disable the temperature driver
			break;
		case 2:
			printf("WARNING: I2C-3 attempted to be accessed, this i2c line \
logic runs on 1.8v and is not really usable!\n");
			break;
		//Case 3 not handled because there is nothing to do with i2c-4, it works normally
		default:
			break;
	}
	
	//Don't initialize twice
	if(neo_i2c_global_freed == 2) {
		for(i = 0; i <= I2CCOUNT; i++) {
			neo_i2c_freed[i] = 2; //Set fds file to not initialized
			neo_i2c_fds[i] = -1; //Set fds to failed
		}
		neo_i2c_global_freed = 0; //Don't run until all are freed	
	}
	
	if(neo_i2c_freed[adapter] == 2 || neo_i2c_freed[adapter] == 1) {
		char i2c_path[30]; //It should never be more than 30 characters
		sprintf(i2c_path, I2CBASE, adapter); 
		//Compile path with new adapter setting
		
		neo_i2c_fds[adapter] = open(i2c_path, O_RDWR); 
		//Open up i2c file descriptor
		
		if(neo_i2c_fds[adapter] < 0) {
			return NEO_I2C_INIT_ERROR;
		}
		
		neo_i2c_freed[adapter] = 0;
	}
	
	return NEO_OK;
}

/**
 * @brief Sets the slave address to use on the i2c adapter
 * 
 * Sets the slave address on the specified i2c adapter such as 0x4F. You must
 * call this method every time you want to change the slave address.
 *
 * @param adapter The physical hardware i2c adapter number
 * @param addr the slave address (ex: 0x20)
 * @return NEO_OK/NEO_I2C_ADDR_ERROR/NEO_I2C_ADAPTER_ERROR when it failed to init
 * 
 * @note Every read and write will now be sent to this address (Only on the specified adapter)
 */
int neo_i2c_set_addr(int adapter, int addr) {
	adapter -= I2CSHIFT;
	//Make sure the address is between 0 and 255
	if(addr < 0x00 || addr > 0xFF) {
		return NEO_I2C_ADDR_ERROR;
	}
	
	//Double check to see if all options are valid and the adapter has been opened
	if(adapter < 0 || adapter > I2CCOUNT
		 || (neo_i2c_freed[adapter] != 0 
		 && neo_i2c_freed[adapter] != 1)) return NEO_I2C_ADAPTER_ERROR;
	
	//Try telling the adapter to use this i2c address (Even if builtin driver is using it)
	if(ioctl(neo_i2c_fds[adapter], I2C_SLAVE_FORCE, addr) < 0) {
		return NEO_I2C_ADDR_ERROR; //Return the adapter failed to set the address
	}
	
	neo_i2c_freed[adapter] = 1; //Set flag to address is set
	
	return NEO_OK; //Return the status is okay
}

/**
 * @brief Read from the slave addr 
 * 
 * On the assigned adapter this will read from the slave address. And place
 * it into the buffer, you must specify the size to read
 *
 * @param adapter The physical hardware i2c adapter number
 * @param buf The uchar buffer to store the results in
 * @param bufsize The amount to read from the i2c line
 * @return NEO_OK/NEO_I2C_READ_ERROR/NEO_I2C_ADAPTER_ERROR when failed to read
 * @see neo_i2c_set_addr() to set the slave address
 */
int neo_i2c_read(int adapter, unsigned char *buf, int bufsize) {
	adapter -= I2CSHIFT;
	if(adapter < 0 || adapter > I2CCOUNT
		 || neo_i2c_freed[adapter] != 1) return NEO_I2C_ADAPTER_ERROR;
	//Make sure the i2c address has been set before reading
	if(read(neo_i2c_fds[adapter], buf, bufsize) < 0) return NEO_I2C_READ_ERROR;
	
	//Everything was fine
	return NEO_OK;
}

/**
 * @brief Write to the slave addr 
 * 
 * On the assigned adapter this will write to the slave address. And write
 * the specified amount from the buffer
 *
 * @param adapter The physical hardware i2c adapter number
 * @param buf The uchar buffer to write to the i2c line
 * @param bufsize The amount to write to the i2c line (from the buffer)
 * @return NEO_OK/NEO_I2C_WRITE_ERROR/NEO_I2C_ADAPTER_ERROR when failed to write
 * @see neo_i2c_set_addr() to set the slave address
 */
int neo_i2c_write(int adapter, unsigned char *buf, int bufsize) {
	adapter -= I2CSHIFT;
	if(adapter < 0 || adapter > I2CCOUNT
		 || neo_i2c_freed[adapter] != 1) return NEO_I2C_ADAPTER_ERROR;
	//Make sure the i2c address has been set before writing
	if(write(neo_i2c_fds[adapter], buf, bufsize) < 0) return NEO_I2C_WRITE_ERROR;
	
	//Everything was fine
	return NEO_OK;
}

/*
int neo_i2c_write(int adapter, int reg, char byte_write) {
	char buf_write[5]; //Careful over haul
	
	buf_write[0] = reg; //Set device register
	buf_write[1] = byte_write; //Set the first byte to write

	if(write(neo_i2c_fds[adapter], buf_write, 2) != 2) {
		return NEO_I2C_WRITE_ERROR;
	}
	return NEO_OK;
}*/

/**
 * @brief Reads a specific registry from the i2c line
 * 
 * On the assigned adapter this will read from the slave address, at a 
 * specific registry. And placeit into the buffer, you must specify the
 * size to read
 *
 * @param adapter The physical hardware i2c adapter number
 * @param reg The registry address to access such as 0x10
 * @param buf The uchar buffer to store the results in
 * @param bufsize The amount to read from the i2c line
 * @return NEO_OK/NEO_I2C_READ_ERROR/NEO_I2C_WRITE_ERROR/NEO_I2C_ADAPTER_ERROR when failed to read
 * @see neo_i2c_set_addr() to set the slave address
 */
int neo_i2c_read_reg(int adapter, __uint16_t reg, unsigned char *buf, 
						int bufsize) {
	unsigned char reg_buf[2]; //Write the reg to read
	int ret; //Capture return of the registry
	
	reg_buf[0] = (reg >> 0) & 0xFF; //Read from this registry
	reg_buf[1] = (reg >> 8) & 0xFF; //Max byte shift reg
	
	ret = neo_i2c_write(adapter, reg_buf, 2); //Write 2 bytes to i2c line
	if(ret != NEO_OK) return ret; //Failed to set reg, return error
	
	return neo_i2c_read(adapter, buf, bufsize); //Return the code of the read
}

/**
 * @brief Free the specified i2c adapter
 * 
 * This will release the i2c adapter that is specified, if it was never
 * initialized or it was already freed then NEO_OK will be returned
 *
 * @param adapter The physical hardware i2c adapter number
 * @return NEO_OK or NEO_I2C_ADAPTER_ERROR when failed to release
 */
int neo_i2c_free(int adapter) {
	adapter -= I2CSHIFT;
	//Shift the adapter based on the real board number
	if(adapter < 0 || adapter > I2CCOUNT) return NEO_I2C_ADAPTER_ERROR;	

	if(neo_i2c_freed[adapter] != 2) { //Make sure it's not already freed
		//Check for any closing error then return that error
		neo_i2c_freed[adapter] = 2; //Set the adapter flag to freed
		if(close(neo_i2c_fds[adapter] < 0)) return NEO_I2C_ADAPTER_ERROR;
	}
	return NEO_OK;
}

/**
 * @brief Frees all of the i2c lines
 * 
 * Releases all of the i2c lines. Even if they were not initialized
 * it's better to call this at the end of your code to make sure it's done
 * and you can reuse the i2c drivers.
 *
 * @return NEO_OK or NEO_I2C_ADAPTER_ERROR when failed to release one of the i2c lines
 */
int neo_i2c_free_all() {
	int i; //Loop count
	int fail; //The error code to return
	
	fail = NEO_OK; //Set default return
	
	for(i = I2CSHIFT; i < I2CCOUNT; i++) { //Release only shifted ports
		int t_ret = neo_i2c_free(i); //Try freeing the individual i2c lines
		if(t_ret != NEO_OK) fail = t_ret; //Set fail flag to the error code
	}
	
	//Set the global freed to reset the i2c arrays since all are deinitialized
	neo_i2c_global_freed = 2;
	
	return fail;
}

