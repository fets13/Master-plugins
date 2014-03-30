#include "i2c.h"
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <wiringPi.h>
#include <stdexcept>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "logging.h"

using namespace std;
using namespace ydle;


//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
I2C::I2C()
{
	mFd = -1 ;
}
I2C::~I2C()
{
	if (mFd >= 0) {
		close (mFd) ;
		mFd = -1 ;
	}
}
//
// ----------------------------------------------------------------------------
/**
	   Routine: Init I2C object
	   Inputs:  
	   	dev : device name
		addr : i2c device address
		flag : specify if i2c master or slave

	   Outputs:

 */
// ----------------------------------------------------------------------------
void I2C::Init (const char *dev, int addr, int flag)
{
	mI2cDevice = dev ;
	if ((mFd = open(dev, O_RDWR)) < 0) {					// Open port for reading and writing
//FETS			YDLE_FATAL << "Failed to open i2c port : " << mI2cDevice;
		string msg = "Failed to open i2c port : " ;
		msg += dev;
  		throw std::runtime_error(msg);
		return ;
	}
	if (ioctl(mFd, flag, mI2cAddr) < 0) {					// Set the port options and set the address of the device we wish to speak to
//FETS			YDLE_FATAL << "Unable to get bus access to talk to slave : " << mI2cAddr;
		string msg = "Unable to get bus access to talk to slave : " + mI2cAddr;
  		throw std::runtime_error(msg);
		return ;
	}
}

// ----------------------------------------------------------------------------
/**
	   Routine: Write data on i2c bus
	   Inputs:  
	   	reg : registry number
		length : data size to send
		data : data to send

	   Outputs:

 */
// ----------------------------------------------------------------------------
int I2C::Write (uint8_t cmd, uint8_t length, uint8_t * data)
{
	return i2c_smbus_write_i2c_block_data(mFd, cmd, length, data);  // adresse, numero de registre, taille, byte array
}

int I2C::Read (uint8_t cmd, uint8_t length, uint8_t * data)
{
	return i2c_smbus_read_i2c_block_data(mFd, cmd, length, data);		// adresse I2C, registre à lire, byte array de stockage des info reçues
}
