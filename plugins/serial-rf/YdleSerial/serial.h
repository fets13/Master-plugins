
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "ISerial.h"
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#define USE_SOFTSERIAL
#ifdef USE_SOFTSERIAL
#define	MY_SERIAL	SoftwareSerial
#else
#define	MY_SERIAL	HardwareSerial
#endif // USE_SOFTSERIAL

typedef enum {
	MSG_NORMAL = 0
		, MSG_ACK_OK
		, MSG_ACK_NOK
		, MSG_COUNT
} tMsgType ;
typedef struct SerialHeader
{
	uint8_t	begin ;
	uint8_t	type ;	// 0: message, 1:ACK_OK, 2:ACK_NOK
	uint8_t	size ;
	uint8_t	crc ;
} SerialHeader ;




#define SERIAL_MSG_MAX	64
#define SERIAL_MSG_BEGIN	0xAA


class RS232 : public ISerial
{
public:
	RS232 () ;
	RS232 (int rx, int tx, unsigned long baudrate) ;
	virtual bool	ReadBuf (puint8_t buf, int size, int * aSize= NULL) {
	}
	virtual bool	SendBuf(puint8_t buf, int size) ;
	bool			SendBufAgain() ;
	bool			IsReady2Send () ;
	void			ResetInput () ;
	bool			Read () ;
	SerialHeader *	GetHeaderIn() { return &mHeaderIn ; }
	uint8_t			GetMsgType() { return mHeaderIn.type ; }
	uint8_t			GetMsgSize() { return mHeaderIn.size ; }
	puint8_t		GetDataIn() { return mTabIn ; }
	void			SendAck (uint8_t status) ;
	MY_SERIAL *		GetSoftSerial() {return mpSerial ; }
	bool			Manager () ;
private:
	MY_SERIAL *	mpSerial ;
	SerialHeader mHeaderIn, mHeaderOut ;
	uint8_t		mTabIn[SERIAL_MSG_MAX] ;
	uint8_t		mTabOut[SERIAL_MSG_MAX] ;
	uint8_t		mCpt ;
	bool		mInprogress ;
	bool		mWaitAck ;
	uint32_t	mTimeAck ;
	bool		mSomething2Send ;
} ;
#endif __SERIAL_H__
