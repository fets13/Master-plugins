#ifndef __I_SERIAL_H__
#define __I_SERIAL_H__
#include <stdio.h>
typedef unsigned char uint8_t ;
typedef uint8_t * puint8_t ;

class ISerial
{
public:
	virtual ~ISerial() {} 
	virtual bool ReadBuf (puint8_t, int, int*a=NULL) = 0 ;
	virtual bool SendBuf (puint8_t, int) = 0 ;
} ;

#endif // __I_SERIAL_H__
