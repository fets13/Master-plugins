#ifndef I2C_H_
#define I2C_H_

#include <string>
#include <linux/i2c-dev.h>

namespace ydle {

class I2C
{
public:
	I2C () ;
	~I2C () ;
	void	Init (const char *dev, int addr, int flag = I2C_SLAVE) ;
	int		Write (uint8_t reg, uint8_t length, uint8_t * data) ;
	int		Read (uint8_t reg, uint8_t length, uint8_t * data) ;

private:
	int			mFd ;				// I2C file descriptor
	int			mI2cAddr;			// raspberry (slave) i2c bus address 
	std::string	mI2cDevice;			// i2c device name

} ;


}; //namespace ydle

#endif // I2C_H_
