#include "protocol.h"
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <wiringPi.h>
#include <stdexcept>

//FETS	#include "logging.h"
#include "ydle-log.h"
#include "Float.h"
#include "crc.h"


using namespace std;
using namespace ydle;

#define TIME_OUT_ACK  5000000 //microsecondl


extern void scheduler_realtime();
extern void scheduler_standard ();

protocolI2CRF * protocolI2CRF::mpProto ;

//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolI2CRF::protocolI2CRF()
{
	mpProto = this ;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: Destructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolI2CRF::~protocolI2CRF()
{
}

//
// ----------------------------------------------------------------------------
/**
	   Function: initialisation
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
void protocolI2CRF::initialisation()
{
	_mutexSynchro = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // Mutex used to prevent reading RF when we Sending something

	debugActivated = false;
	m_rx_done = false;
	long speed = 1000;
	t_per = 1000000/speed;

}


// ----------------------------------------------------------------------------
/**
	   Function: transmit
	   Inputs:  

	   Outputs: 

// 			Transmit Message
 */
// ----------------------------------------------------------------------------
void protocolI2CRF::transmit(bool bRetransmit)
{
	int a = 0;
	uint8_t trame[30];

	itob(m_sendframe.receptor,0,8);
	itob(m_sendframe.sender,8,8);
	itob(m_sendframe.type,16,3);
	itob(m_sendframe.taille,19,5);
	for(a=0;a<m_sendframe.taille-1;a++)
	{
		itob(m_sendframe.data[a],24+(8*a),8);
	}

	memcpy(trame, &m_sendframe, (m_sendframe.taille)+3);

	// wait atmega available until sending i2c data
	while(IsRFDeviceBusy()) {
		// Wait
//FETS			delay(2*YDLE_TPER);
		delay(200);
	}

	mI2C.Write(0x01, (m_sendframe.taille)+3, trame);  // adresse, numero de registre, taille, byte array
	cout << "I2C Transmission done" << endl;
	printFrame(m_sendframe);

	//unLock reception when we finished
	pthread_mutex_unlock(&_mutexSynchro);
}




// ----------------------------------------------------------------------------
/**
	   Function: addCmd
	   Inputs:  int type type of data
				int data

	   Outputs: 

 */
// ----------------------------------------------------------------------------
void protocolI2CRF::addCmd(int type,int data)
{
	m_sendframe.data[m_sendframe.taille]=type<<4;
	m_sendframe.data[m_sendframe.taille+1]=data;
	m_sendframe.taille+=2;
}

// ----------------------------------------------------------------------------
/**
	   Function: addData
	   Inputs:  int type type of data
				int data

	   Outputs: 

 */
// ----------------------------------------------------------------------------
void protocolI2CRF::addData(int type,int data)
{
	int oldindex = m_sendframe.taille;


	switch (type)
	{
	// 4 bits no signed
	case DATA_ETAT :
		if (m_sendframe.taille<29)
		{
			m_sendframe.taille++;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=data&0x0f;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	

		// 12 bits signed
	case DATA_DEGREEC:
	case DATA_DEGREEF :
	case DATA_PERCENT :
	case DATA_HUMIDITY:
		if (m_sendframe.taille<28)
		{
			m_sendframe.taille+=2;
			m_sendframe.data[oldindex]=type<<4;
			if (data <0)
			{
				data=data *-1;
				m_sendframe.data[oldindex]^=0x8;
			}
			m_sendframe.data[oldindex]+=(data>>8)&0x0f;
			m_sendframe.data[oldindex+1]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";

		break;	

		// 12 bits no signed
	case DATA_DISTANCE:
	case DATA_PRESSION:
		if (m_sendframe.taille<28)
		{
			m_sendframe.taille+=2;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=(data>>8)&0x0f;
			m_sendframe.data[oldindex+1]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	

		// 20 bits no signed
	case DATA_WATT  :
		if (m_sendframe.taille<27)
		{
			m_sendframe.taille+=3;
			m_sendframe.data[oldindex]=type<<4;
			m_sendframe.data[oldindex]+=(data>>16)&0x0f;
			m_sendframe.data[oldindex+1]=(data>>8)&0xff;
			m_sendframe.data[oldindex+2]=data;
		}
		else
			YDLE_WARN << "invalid trame len in addData";
		break;	
	}
}

// ----------------------------------------------------------------------------
/**
	   Function: listenSignal
	   Inputs:  

	   Outputs: 

//thread reading RX PIN
 */
// ----------------------------------------------------------------------------
void protocolI2CRF::listenSignal()
{
	scheduler_realtime();

	while(1) {

		if(isDone() == true) {
			setDone(false);
			YDLE_DEBUG << "New frame ready to be sent :";
			printFrame(m_receivedframe);
			Notify (&m_receivedframe) ; // notiy new frame received

		} else if (IsI2CDataAvailable () && !IsRFDeviceBusy() && isDone() == false) {
printf ("listensignal : dataavail:%d devicebusy:%d done:%d\n", 
		IsI2CDataAvailable (), !IsRFDeviceBusy(), isDone()) ;
			uint8_t trame[30];
			memset(&trame, 0x0, sizeof(trame));
			mI2C.Read(0x00, 30, trame);		// adresse I2C, registre à lire, byte array de stockage des info reçues
			memcpy(&m_receivedframe, &trame, (trame[3]&= 0x1F)+3);
			setDone(true);
		} else {
			//TODO à changer d'urgence
			delay(5);
		}
	}
	// Code jamais atteint....
	scheduler_standard();
}

void* protocolI2CRF::listenSignal(void* pParam)
{
	YDLE_DEBUG << "Enter in thread listen";
	protocolI2CRF* parent=(protocolI2CRF*)pParam;
	
//	scheduler_realtime();

	if (pParam)
	{
		parent->listenSignal() ;
	}
	YDLE_INFO << "Exit of thread listen";

//	scheduler_standard ();

	return NULL;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: power2()
	   Inputs:  power

	   Outputs:

    Calcul le nombre 2^chiffre indiqué, fonction utilisé par itob pour la conversion decimal/binaire
 */
// ----------------------------------------------------------------------------
unsigned long protocolI2CRF::power2(int power)
{
	unsigned long integer=1;
	for (int i=0; i<power; i++)
	{
		integer*=2;
	}
	return integer;
}


// ----------------------------------------------------------------------------
/**
	   Routine: itob()
	   Inputs:  integer
	   start,
	   length

	   Outputs:


 */
// ----------------------------------------------------------------------------
void protocolI2CRF::itob(unsigned long integer, int start, int length)
{
	for (int i=0; i<length; i++)
	{
		int pow2 = 1 << (length-1-i) ;
		m_FrameBits[start + i] = ((integer & pow2) != 0);
	}
}

// ----------------------------------------------------------------------------
/**
	   Routine: dataToFrame
	   Inputs:  Receiver
	   Transmitter,
	   type
		data
	   Outputs:


 */
// ----------------------------------------------------------------------------
void protocolI2CRF::dataToFrame(unsigned long Receiver, unsigned long Transmitter, unsigned long type)
{
	memset(m_sendframe.data,0,sizeof(m_sendframe.data));
	m_sendframe.sender=Transmitter;
	m_sendframe.receptor=Receiver;
	m_sendframe.type=type;
	m_sendframe.taille=0;
	m_sendframe.crc=0;
} 


// ----------------------------------------------------------------------------
/**
	   Routine: printFrame
	   Inputs:  


	   // log Frame
 */
// ----------------------------------------------------------------------------
void protocolI2CRF::printFrame(Frame_t & trame)
{
	// if debug
	if(debugActivated) {
		char sztmp[255];
		YDLE_DEBUG << "Emetteur : " << (int)trame.sender;
		YDLE_DEBUG << "Recepteur :" << (int)trame.receptor;
		YDLE_DEBUG << "Type :" << (int)trame.type;
		YDLE_DEBUG << "Taille :" << (int)trame.taille;
		YDLE_DEBUG << "CRC :" << (int)trame.crc;

		sprintf(sztmp,"Data Hex: ");
		for (int a=0;a<trame.taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,trame.data[a]);
		YDLE_DEBUG << sztmp;

		sprintf(sztmp,"Data Dec: ");
		for (int a=0;a<trame.taille-1;a++)
			sprintf(sztmp,"%s %d",sztmp,trame.data[a]);
		YDLE_DEBUG << sztmp;
	}
}

// ----------------------------------------------------------------------------
/**
	   Routine: debugMode
	   Inputs:  


	   // Activate debug mode
 */
// ----------------------------------------------------------------------------
void protocolI2CRF::debugMode(bool mode)
{
	debugActivated = mode;
}

// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
void protocolI2CRF::setDone(bool bvalue)
{
	m_rx_done = bvalue;
}

// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
bool protocolI2CRF::isDone()
{
	return m_rx_done;
}


void protocolI2CRF::Start()
{
	// Start listen thread
	printf ("\t\txreateThread\n") ;
	if (pthread_create(&_thread, NULL, protocolI2CRF::listenSignal, this) < 0) {
  		throw runtime_error("Can't start ListenRF thread");

	}
	delay (1000) ;
	printf ("\t\tmyInterrupt\n") ;
	if (wiringPiISR (mPinDataAvailable, INT_EDGE_RISING, &protocolI2CRF::myInterrupt) < 0)
	{
  		throw runtime_error("Unable to setup ISR: %s\n");
	}
	printf ("\t\tmyInterrupt Fin\n") ;

	
}

void protocolI2CRF::InitPlugin()
{
	SettingsParser * pSettings = SettingsParser::Instance() ;
	mPinReceiving		= pSettings->Int("i2crf:rf_working_pin");
	mPinDataAvailable	= pSettings->Int("i2crf:data_ok_pin");
	string i2c_dev		= pSettings->Str("i2crf:device");
	int i2c_addr			= pSettings->Int("i2crf:addr");
	YDLE_DEBUG << "Init protocolI2CRF through I2C";
	YDLE_DEBUG << "	 RFReveiving: " << mPinReceiving;
	YDLE_DEBUG << "	 DataAvailable: " << mPinDataAvailable;
	YDLE_DEBUG << "	 I2C-ADDR: " << i2c_addr;
	YDLE_DEBUG << "	 I2C-DEV: " << i2c_dev;

	mI2C.Init (i2c_dev.c_str(), i2c_addr) ;
	pinMode(mPinDataAvailable, INPUT);
	pinMode(mPinReceiving, INPUT);
	initialisation();

}

void protocolI2CRF::SendMsg (Frame_t & frame)
{
	memcpy (&m_sendframe, &frame, sizeof (Frame_t)) ;
	transmit () ;
}

void protocolI2CRF::receive()
{
	if(IsRFDeviceBusy()) {
		cout << "---------- wait... ----------" << endl;
		cout << "IsRFDeviceBusy : " << IsRFDeviceBusy() << endl;

		timespec time;
		time.tv_sec = 0;
		time.tv_nsec = 10000;

		nanosleep(&time, NULL);
		receive();
	} else {
		delay(10);
		cout << "receive" << endl;
		uint8_t trame[30];
		memset(&trame, 0x0, sizeof(trame));
		mI2C.Read(0x00, 30, trame);		// adresse I2C, registre à lire, byte array de stockage des info reçues
		delay(10);
		memcpy(&m_receivedframe, &trame, (trame[3]&= 0x1F)+3);
		YDLE_DEBUG << "New frame ready to be sent :";
		printFrame(m_receivedframe);
		Notify (&m_receivedframe) ; // notiy new frame received
	}
}

bool protocolI2CRF::IsRFDeviceBusy()
{
	return digitalRead(mPinReceiving) ;
}
bool protocolI2CRF::IsI2CDataAvailable ()
{
	return digitalRead(mPinDataAvailable) ;
}
// ----------------------------------------------------------------------------
/**
 Routine: myInterrupt()
 Inputs:

 Outputs:


 */
// ----------------------------------------------------------------------------
void protocolI2CRF::myInterrupt (void)
{
	mpProto->receive() ;
}

