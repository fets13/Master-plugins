#ifndef ProtocolI2CRF_H_
#define ProtocolI2CRF_H_

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <list>
#include "INode.h"

using namespace std;


#include "IProtocol.h"

#include "i2c.h"

namespace ydle {


class protocolI2CRF : public IProtocol
{

public:

	// Ecoute le récepteur pour l'arrivée d'un signal
	static void* listenSignal(void* pParam);
	void		listenSignal();

	protocolI2CRF();

	~protocolI2CRF();

	// ----------------------------------
	// - INTERFACE IProtocol
	// ----------------------------------
	// activate/desaxctivate debug
	virtual void debugMode(bool mode= true) ;
	virtual void InitPlugin () ;
	// protocol name
	virtual std::string Name () { return "ProtocolI2C-RF" ; }
	virtual void Start () ;
	virtual void SendMsg (Frame_t &) ;
	pthread_t _thread ;


	bool isDone();

	void setDone(bool bvalue);

	// Crée une trame avec les infos données en paramètre
	void dataToFrame(unsigned long recepteur, unsigned long emetteur, unsigned long type);

	// extract any type of data from receivedsignal

	// add TYPE_ETAT data
	void addData(int type,int data);

	// add TYPE_CMD data
	void addCmd(int type,int data);

	// Envoie des verrous et des bits formant une trame
	void transmit(bool bRetransmit = false);
	
	void receive();
	
	
protected:

	// Permet l'initialisation du node
	void initialisation();

	// Calcule 2^"power"
	unsigned long power2(int power);

	// Conversion décimal vers tableau de binaires
	void itob(unsigned long integer, int start, int length);


	// Affiche le contenue des trames reçues
	void printFrame(Frame_t & trame);

	bool	IsI2CDataAvailable () ;
	bool	IsRFDeviceBusy () ;
	static void myInterrupt (void) ;


private:
	int mPinReceiving;		// receiving data in progress on atmega
	int mPinDataAvailable;	// data are available on atmega
	I2C		mI2C ;
	static protocolI2CRF *		mpProto ;


	// Le tableau contenant la trame 272 BITS MAX
	bool m_FrameBits[272]; 

	// On déclare les structures 
	Frame_t m_receivedframe;  // received frame

	Frame_t m_sendframe;	 // send frame	

	// Pour activer le mode débug
	bool debugActivated ;

	// La période d'un bit en microseconds
	long t_per ;

	// Si le message est complet
	bool m_rx_done;
	
   	// Mutex used to prevent reading RF when we Sending somethin // Mutex used to prevent reading RF when we Sending somethingg
	pthread_mutex_t _mutexSynchro ;

};

}; //namespace ydle

#endif // ProtocolI2CRF_H_
