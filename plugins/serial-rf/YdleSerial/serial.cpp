#include "serial.h"
#include "crc.h"
#include "common.h"
#include <Arduino.h>

#define LED_ON digitalWrite(13,HIGH)
#define LED_OFF digitalWrite(13,LOW)

#define MAX_WAIT_ACK	5000

RS232::RS232 (int rx, int tx, unsigned long baudrate)
{
//FETS		mTabIn = new uint8_t[SERIAL_MSG_MAX] ;
	ResetInput() ;
#ifdef USE_SOFTSERIAL
	mpSerial = new SoftwareSerial	(rx,tx) ;
	pinMode (rx, INPUT) ;
	pinMode (tx, OUTPUT) ;
#else
	mpSerial = &Serial ;
#endif // USE_SOFTSERIAL
	mpSerial->begin(baudrate);
	mSomething2Send = false ;
	mWaitAck = false ;

//FETS		delay (1000) ;
}

void RS232::SendAck(uint8_t status)
{
	SerialHeader header ;
	header.begin = SERIAL_MSG_BEGIN ;
	header.size = 0 ;
	header.type = status ;
	header.crc =  0 ;
	mpSerial->write (puint8_t(&header), sizeof(SerialHeader)) ;
	Serial.println ("RS ACK sent");
	
}


bool RS232::SendBuf(puint8_t buf, int size)
{
	if (mWaitAck) return false ;
	mHeaderOut.begin = SERIAL_MSG_BEGIN ;
	mHeaderOut.type = MSG_NORMAL ;
	mHeaderOut.size = size ;
	mHeaderOut.crc =  crc8(buf, size) ;
	LED_ON ;
	memcpy (mTabOut, buf, size) ;
	mSomething2Send = true ;

	return true ;
}

bool RS232::SendBufAgain()
{
	mpSerial->write (puint8_t(&mHeaderOut), sizeof(SerialHeader)) ;
	mpSerial->write (mTabOut, mHeaderOut.size) ;
	mWaitAck = true ;	// we want ACK
	mTimeAck = millis () ;
	return true ;
}


void RS232::ResetInput ()
{
	mCpt = 0 ;
	mInprogress = false ;
}


bool RS232::Read () 
{
	if(mpSerial->available()){
		uint8_t c = mpSerial->read();
//FETS			if (c != 0) {
//FETS				Serial.print ("Read car:0x") ;
//FETS				Serial.print (c,HEX) ;
//FETS				Serial.print (" ") ;
//FETS				Serial.println ((char)c) ;
//FETS			}
		if (!mInprogress && c == SERIAL_MSG_BEGIN)  {
			ResetInput () ;
			D1 ("BEGIN") ;
			mInprogress = true ;
			memset (&mHeaderIn, 0, sizeof (SerialHeader));
		}
		else if (mInprogress) {
			mCpt++ ;
			switch (mCpt) {
				default:
					mTabIn[mCpt-sizeof(SerialHeader)] = c ;
					// end of msg reached
					if ((mCpt - sizeof(SerialHeader)+1) == mHeaderIn.size) {
						// check if crc is good
						uint8_t crc = crc8(mTabIn, mHeaderIn.size) ;
						// check crc
						if (mHeaderIn.crc != crc) {
							Serial.print ("Serial crc incorrect !!!");
							Serial.print (crc, HEX);
							Serial.print ("~");
							Serial.print (mHeaderIn.crc, HEX);
							ResetInput () ;
							return false ;
						}
				D1("GSM");
						// here we have a good message
						mInprogress = false ;
						return true ;
						
					}
//FETS					D1("GSM");
					break ;
				case 1: // 2nd byte = type
					mHeaderIn.type = c ;
					if (c < MSG_NORMAL || c >= MSG_COUNT) {
						D2 ("Serial received type incorrect !!! ", c);
						ResetInput () ;
						return false ;
					}
//FETS						D2arg ("type:", c, HEX) ;
					break ;
				case 2: // 3rd byte = size
					mHeaderIn.size = c ;
					if (c > SERIAL_MSG_MAX) {
						D2arg ("Serial received size incorrect !!! : ", c,HEX);
						ResetInput () ;
					}
//FETS						D2 ("size:", c) ;
					break ;
				case 3: // 4th byte = crc
					mHeaderIn.crc = c ;
//FETS						D2arg ("crc:", c, HEX) ;
					// Cas particulier d'un ACK
					if (mHeaderIn.size == 0 && mHeaderIn.crc == 0) {
						D1 ("ACK") ;
						mInprogress = false ;
						return true ;
					}
					break ;
			}
		}
	}

	return false ;
}

bool RS232::Manager()
{
	// cehck for new incoming message
	if (Read ()) {
		// manage it
		switch (GetMsgType()) {
			case MSG_ACK_OK:
				Serial.println("ack receveived");
				LED_OFF ;
				mWaitAck = false ;
				break ;
			case MSG_ACK_NOK:
				Serial.println("ack KO, send again");
				delay (100) ;
				SendBufAgain () ;
				break ;
			case MSG_NORMAL:
				Serial.println("MSG NORMAL rcv !!!");
				SendAck (MSG_ACK_OK) ;
				return true ;
				break ;
		}
	}
	if (mSomething2Send) {
		mSomething2Send = false ;
		Serial.println("Send MSG");
		SendBufAgain() ;
	}

	if (mWaitAck) {
		// wait too long, send message again
		if ((millis() - mTimeAck) > MAX_WAIT_ACK) {
			// send again the buffer
			Serial.println("Send MSG AGAIN");
			SendBufAgain () ;
		}
	}
	return false ;
}
bool RS232::IsReady2Send ()
{
	return !mWaitAck ;
}
