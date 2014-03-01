#include "protocolRF.h"
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <wiringPi.h>
#include <stdexcept>

#include "logging.h"
#include "Float.h"
#include "crc.h"

using namespace std;
using namespace ydle;

#define TIME_OUT_ACK  5000000 //microsecondl


extern void scheduler_realtime();
extern void scheduler_standard ();



uint8_t protocolRF::computeCrc(Frame_t* frame){
	uint8_t *buf, crc;
	int a,j;

	buf = (uint8_t*)malloc(frame->taille+3);
	memset(buf, 0x0, frame->taille+3);

	buf[0] = frame->sender;
	buf[1] = frame->receptor;
	buf[2] = frame->type << 5;
	buf[2] |= frame->taille;

	for(a=3, j=0 ;j < frame->taille - 1;a++, j++){
		buf[a] = frame->data[j];
	}
	// message size = Sender (1byte) + receptor(1 byte) + type/size (1byte) + data/crc (size bytes)
	// without crc : total size : 3 + taille - 1 = taille+2
	crc = crc8(buf,frame->taille+2);
	free(buf);
	return crc;
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  RX and TX PIN

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::protocolRF(int rx, int tx)
{
	Init (rx, tx) ;
}

void protocolRF::Init(int rx, int tx)
{

	m_pinRx=rx;
	m_pinTx=tx;

	pinMode(m_pinTx, OUTPUT);
	pinMode(m_pinRx, INPUT);
	initialisation();
	YDLE_DEBUG << "Init protocolRF TX: " << m_pinTx;
	YDLE_DEBUG << "Init protocolRF RX: " << m_pinRx;
}


//
// ----------------------------------------------------------------------------
/**
	   Routine: Constructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::protocolRF()
{
}

//
// ----------------------------------------------------------------------------
/**
	   Routine: Destructor
	   Inputs:  

	   Outputs:

 */
// ----------------------------------------------------------------------------
protocolRF::~protocolRF()
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
void protocolRF::initialisation()
{
	_mutexSynchro = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // Mutex used to prevent reading RF when we Sending something

	transmissionType = 0;
	initializedState = false;

	memset(start_bit2,0,sizeof(start_bit2));
	start_bit2[1]=true;
	start_bit2[6]=true;

	debugActivated = false;
	m_sample_value = 0;
	sample_count = 1;
	last_sample_value = 0;
	pll_ramp = 0;
	sample_sum = 0;
	rx_bits = 0;
	t_start = 0;
	rx_active = 0;
	speed = 1000;
	t_per = 1000000/speed;
	f_bit = t_per/8;
	bit_value = 0;
	bit_count = 0;
	sender = 0;
	receptor = 0;
	type = 0;
	parite = false;
	taille = 0;
	memset(m_data,0,sizeof(m_data));
	rx_bytes_count = 0;
	length_ok = 0;
	m_rx_done = 0;
}


// ----------------------------------------------------------------------------
/**
	   Function: sendPair
	   Inputs:  bit value 

	   Outputs: Send  bit with Manchester codage

 */
// ----------------------------------------------------------------------------
void protocolRF::sendPair(bool b) 
{
	sendBit(b);
	sendBit(!b);
}


// ----------------------------------------------------------------------------
/**
	   Function: sendBit
	   Inputs:  bit value 

	   Outputs: 

// 			Send a pulse on PIN
//			1 = t_per µs HIGH
//			0 = t_per µs LOW
 */
// ----------------------------------------------------------------------------
void protocolRF::sendBit(bool b)
{

	if (b) {                       // si "1"
		digitalWrite(m_pinTx, HIGH);   // Pulsation à l'état haut
		delayMicroseconds(t_per);      // t_per
	}
	else {                         // si "0"
		digitalWrite(m_pinTx, LOW);    // Pulsation à l'état bas
		delayMicroseconds(t_per);      // t_per
	}
}

void protocolRF::SendMsg (Frame_t & frame)
{
	memcpy (&m_sendframe, &frame, sizeof (Frame_t)) ;
	transmit () ;
}

// ----------------------------------------------------------------------------
/**
	   Function: transmit
	   Inputs:  

	   Outputs: 

// 			Transmit Message
 */
// ----------------------------------------------------------------------------
void protocolRF::transmit(bool bRetransmit)
{
	int j = 0;
	int a = 0;
	int i = 0;
	uint8_t crc;
	// calcul crc

	m_sendframe.taille++; // add crc BYTE
	crc = computeCrc(&m_sendframe);
	YDLE_DEBUG << "Send ACK";
	m_sendframe.crc = crc;

	itob(m_sendframe.receptor,0,8);
	itob(m_sendframe.sender,8,8);
	itob(m_sendframe.type,16,3);
	itob(m_sendframe.taille,19,5);
	for(a=0;a<m_sendframe.taille-1;a++)
	{
		itob(m_sendframe.data[a],24+(8*a),8);
	}

	itob(m_sendframe.crc,24+(8*a),8);

	// If CMD then wait	 for ACK ONLY IF it's not already a re-retransmit
	if(m_sendframe.type == TYPE_CMD && !bRetransmit) {
		ACKCmd_t newack;

		memcpy(&newack.Frame,&m_sendframe,sizeof(Frame_t));
		newack.iCount=0;
		newack.Time = getTime();
		mListACK.push_back(newack);
	}

	//Lock reception when we end something
	pthread_mutex_lock(&_mutexSynchro);

//	scheduler_realtime();


	// Sequence AGC
	for (int x=0; x < 32; x++) { sendPair(true); }

	for (j=0; j<8; j++) { sendPair(start_bit2[j]); }

	// Send Data
	for(i=0; i<(m_sendframe.taille+3)*8;i++) // data+entete+crc
	{
		sendPair(m_FrameBits[i]);
	}

	digitalWrite(m_pinTx, LOW);

//	scheduler_standard ();

//FETS		if(debugActivated) YDLE_DEBUG << ("end sending");

	//unLock reception when we finished
	pthread_mutex_unlock(&_mutexSynchro);
}

//TODO: Manque la gestion des erreurs!

#if 0
int protocolRF::extractData(int position, float & data){

	float half =  (m_receivedframe.data[position] << 8) | m_receivedframe.data[position+1];
	for(int i = 16; i>=0; --i){
		bool t = (1<<i) & half ;
		std::cout << t;
	}
	data = Float16To32 (half) ;
	return 0;
}

int protocolRF::extractData(int position, int & data){
	uint16_t b;
	b = (m_receivedframe.data[position] << 8) | m_receivedframe.data[position+1];
	data = b;
	return 0;
}

// ----------------------------------------------------------------------------
/**
	   Function: extractData
	   Inputs:  int index: index de la value recherche (0..29)
				int itype: en retour type de la value
				int ivalue: en retour, value
				pBuffer : bufer to search in , if NULL then use m_receivedframe
	   Outputs: 1 value trouve,0 non trouve,-1 no data

 */
// ----------------------------------------------------------------------------
int protocolRF::extractData(int index,int &itype,int &ivalue,uint8_t* pBuffer /*=NULL*/,int ilen /*=0*/)
{
	uint8_t* ptr;
	bool bifValueisNegativ=false;
	int iCurrentValueIndex=0;
	bool bEndOfData=false;
	int  iLenOfBuffer = 0;
	int  iModifType=0;
	int  iNbByteRest=0;

	if(pBuffer==NULL)
	{
		ptr=m_receivedframe.data;
		iLenOfBuffer=m_receivedframe.taille;
	}	
	else
	{
		iLenOfBuffer=ilen;
		ptr=pBuffer;
	}	

	if(iLenOfBuffer <2) // Min 1 byte of data with the 1 bytes CRC always present, else there is no data
		return -1;

	while (!bEndOfData)
	{
		itype=(uint8_t)*ptr>>4;
		bifValueisNegativ=false;

		// This is a very ugly code :-( Must do something better
		if(m_receivedframe.type==TYPE_CMD)
		{
			// Cmd type if always 12 bits signed value
			iModifType=DATA_DEGREEC;
		}
		else if(m_receivedframe.type==TYPE_ETAT)
		{
			iModifType=itype;
		}
		else
		{
			iModifType=itype;
		}

		switch(iModifType)
		{
		// 4 bits no signed
		case DATA_ETAT :
			ivalue=*ptr&0x0F;
			ptr++;
			iNbByteRest--;
			break;	

			// 12 bits signed
		case DATA_DEGREEC:
		case DATA_DEGREEF :
		case DATA_PERCENT :
		case DATA_HUMIDITY:
			if(*ptr&0x8)
				bifValueisNegativ=true;
			ivalue=(*ptr&0x07)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			if(bifValueisNegativ)
				ivalue=ivalue *(-1);
			iNbByteRest-=2;
			break;	

			// 12 bits no signed
		case DATA_DISTANCE:
		case DATA_PRESSION:
			ivalue=(*ptr&0x0F)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=2;
			break;	

			// 20 bits no signed
		case DATA_WATT  :
			ivalue=(*ptr&0x0F)<<16;
			ptr++;
			ivalue+=(*ptr)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			iNbByteRest-=3;
			break;	
		}

		if (index==iCurrentValueIndex)
			return 1;

		iCurrentValueIndex++;

		if(iNbByteRest<1)
			bEndOfData =true;;
	}

	return 0;	
}
#endif




// ----------------------------------------------------------------------------
/**
	   Function: addCmd
	   Inputs:  int type type of data
				int data

	   Outputs: 

 */
// ----------------------------------------------------------------------------
void protocolRF::addCmd(int type,int data)
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
void protocolRF::addData(int type,int data)
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
	   Function: pll
	   Inputs:  

	   Outputs: 

// Called 4 times for each bit period
// This function try to syncronize signal
 */
// ----------------------------------------------------------------------------
void protocolRF::pll()
{
	sample_count ++;

	// On additionne chaque sample et on incrémente le nombre du prochain sample
	if (m_sample_value)
	{
		sample_sum++;
	}

	// On vérifie s'il y a eu une transition de bit
	if (m_sample_value != last_sample_value)
	{
		// Transition, en avance si la rampe > 40, en retard si < 40
		if(pll_ramp < 80)
		{
			pll_ramp += 11; 
		} else
		{
			pll_ramp += 29;
		}
		last_sample_value = m_sample_value;
	}
	else
	{
		// Si pas de transition, on avance la rampe de 20 (= 80/4 samples)
		pll_ramp += 20;
	}

	// On vérifie si la rampe à atteint son maximum de 80
	if (pll_ramp >= 160)
	{
		//	log ("pll ok Bits:",rx_bits);
		t_start = micros();
		// On ajoute aux 16 derniers bits reçus rx_bits, MSB first
		// On stock les 16 derniers bits
		rx_bits <<= 1;

		// On vérifie la somme des samples sur la période pour savoir combien était à l'état haut
		// S'ils étaient < 2, on déclare un 0, sinon un 1;
		if (sample_sum >= 5)
		{
			rx_bits |= 0x1;
			bit_value = 1;
			//	   		digitalWrite(pinCop, HIGH);
		}
		else
		{
			rx_bits |= 0x0;
			bit_value = 0;
			//	   		digitalWrite(pinCop,LOW);
		}
		pll_ramp -= 160; // On soustrait la taille maximale de la rampe à sa valeur actuelle
		sample_sum = 0; // On remet la somme des samples à 0 pour le prochain cycle
		sample_count = 1; // On ré-initialise le nombre de sample



		// Si l'on est dans le message, c'est ici qu'on traite les données
		if (rx_active)
		{
			//			if(debugActivated)
			//				log("message : ",rx_bytes_count);

			bit_count ++;

			// On récupère les bits et on les places dans des variables
			// 1 bit sur 2 avec Manchester
			if (bit_count % 2 == 1)
			{
				if (bit_count < 16)
				{
					// Les 8 premiers bits de données
					receptor <<= 1;
					receptor |= bit_value;
				}
				else if (bit_count < 32)
				{
					// Les 8 bits suivants
					sender <<= 1;
					sender |= bit_value;
				}
				else if (bit_count < 38)
				{
					// Les 3 bits de type
					type <<= 1;
					type |= bit_value;
				}
				else if (bit_count < 48)
				{
					// Les 5 bits de longueur de trame
					rx_bytes_count <<= 1;
					rx_bytes_count |= bit_value;
				}
				else if ((bit_count-48) < (rx_bytes_count * 16))
				{
					length_ok = 1;
					m_data[(bit_count-48)/16] <<= 1;
					m_data[(bit_count-48)/16]|= bit_value;
				}
			}

			// Quand on a reçu les 24 premiers bits, on connait la longueur de la trame
			// On vérifie alors que la longueur semble logique	
			if (bit_count >= 48)
			{
				// Les bits 19 à 24 informent de la taille de la trame
				// On les vérifie car leur valeur ne peuvent être < à 1 et > à 31
				if (rx_bytes_count < 1 || rx_bytes_count > 31)
				{
					if(debugActivated)
						YDLE_DEBUG << "error!" << rx_bytes_count;

					// Mauvaise taille de message, on ré-initialise la lecture
					rx_active = false;
					sample_count = 1;
					bit_count = 0;
					length_ok = 0;
					sender = 0;
					receptor = 0;
					type = 0;
					taille = 0;
					memset(m_data,0,sizeof(m_data));
					t_start = micros();
					return;
				}
			}

			// On vérifie si l'on a reçu tout le message
			if ((bit_count-48) >= (rx_bytes_count*16) && (length_ok == 1))
			{
				if(debugActivated)
					YDLE_DEBUG <<  ("complete");

				rx_active = false;
				m_receivedframe.sender = sender;
				m_receivedframe.receptor = receptor;
				m_receivedframe.type = type;
				m_receivedframe.taille = rx_bytes_count;
				memcpy(m_receivedframe.data,m_data,rx_bytes_count-1); // copy data len - crc

				// crc calcul
				m_receivedframe.crc = computeCrc(&m_receivedframe);

				if(m_data[rx_bytes_count-1] != m_receivedframe.crc) {	
					if(debugActivated)
						YDLE_WARN << "crc error !!!";
				}
				else
				{
					m_rx_done = true;
					/*float test;int test2;
					this->printFrame(m_receivedframe);
					this->extractData(0, test);
					this->extractData(2, test2);
					YDLE_DEBUG << "Value received :" << test << " " << test2;*/
				}
				length_ok = 0;
				sender = 0;
				receptor = 0;
				type = 0;
				taille = 0;
				memset(m_data,0,sizeof(m_data));
			}

		}

		// Pas dans le message, on recherche l'octet de start
		else
		{
			if (rx_bits == 0x06559)
			{
				if(debugActivated)
					YDLE_DEBUG << ("start received");
				// Octet de start, on commence à collecter les données
				rx_active = true;
				bit_count = 0;
				rx_bytes_count = 0;
			}
		}
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
void protocolRF::listenSignal()
{
	int err = 0;
	scheduler_realtime();
	timespec time;

	while(1)
	{
		// Si le temps est atteint, on effectue une mesure (sample) puis on appelle la PLL
		//  *-------  1 CAS, on calcul le temps a attendre entre 2 lecture ------
		int tempo=(t_start + (sample_count * f_bit)) -micros() -2;

		if(tempo<5) {
			tempo=5; // la fonction delayMicroseconds n'aime pas la valeur negative
		}
		time.tv_sec = 0;
		time.tv_nsec = tempo * 1000;
		nanosleep(&time, NULL);

		// try to received ONLY if we are not currently sending something
		err=pthread_mutex_lock(&_mutexSynchro);
		if( err== 0)
		{		  
			if(isDone())
			{
				setDone(false);
			}

			m_sample_value = digitalRead(m_pinRx);

			pll();

			// if a full signal is received
			if(isDone())
			{
				// If it's a ACK then handle it
				if(m_receivedframe.type == TYPE_ACK)
				{
					std::list<protocolRF::ACKCmd_t>::iterator i;
					for(i=mListACK.begin(); i != mListACK.end(); ++i)
					{
						if(m_receivedframe.sender == i->Frame.receptor
								&& m_receivedframe.receptor == i->Frame.sender)
						{
							YDLE_DEBUG << "Remove ACK from pending list";
							i=mListACK.erase(i);
							break; // remove only one ACK at a time.
						}
					}
				}
				else if(m_receivedframe.type == TYPE_ETAT_ACK)
				{
					// Send ACK	
					dataToFrame(m_receivedframe.sender,m_receivedframe.receptor,TYPE_ACK);				
					delay (250);
					// Sequence AGC supplémentaire nécessaire
					for (int x=0; x < 32; x++)
					{
						sendPair(true);
					}
					transmit(0);
					// notiy new frame received
					YDLE_DEBUG << "New frame (TYPE_ETAT_ACK) ready to be sent :";
//FETS						printFrame(m_receivedframe);
					Notify (&m_receivedframe) ;
				}
				else //else send it to IHM
				{
					YDLE_DEBUG << "New frame ready to be sent :";
					printFrame(m_receivedframe);
					// notiy new frame received
					Notify (&m_receivedframe) ;
				}
			}

			// Let's Send thread
			pthread_mutex_unlock(&_mutexSynchro);
		}
		else
		{
			YDLE_WARN << "error acquire mutex" << err;
		}
		// check if we need re-transmit	
		checkACK();
	}	
	// Code jamais atteint....
	scheduler_standard();
}

void* protocolRF::listenSignal(void* pParam)
{
	YDLE_DEBUG << "Enter in thread listen";
	protocolRF* parent=(protocolRF*)pParam;
	
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
unsigned long protocolRF::power2(int power)
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
void protocolRF::itob(unsigned long integer, int start, int length)
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
void protocolRF::dataToFrame(unsigned long Receiver, unsigned long Transmitter, unsigned long type)
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
void protocolRF::printFrame(Frame_t & trame)
{
	// if debug
	if(debugActivated)
	{
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
void protocolRF::debugMode(bool mode)
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
int protocolRF::getType()
{
	return m_receivedframe.type;
}



// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
int protocolRF::getTaille()
{
	return m_receivedframe.taille;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
uint8_t* protocolRF::getData()
{
	return m_receivedframe.data;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
int protocolRF::isSignal()
{
	return rx_active;
}


// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
void protocolRF::setDone(bool bvalue)
{
	m_rx_done=bvalue;
}

// ----------------------------------------------------------------------------
/**
	   Routine: 
	   Inputs:  


	   // 
 */
// ----------------------------------------------------------------------------
bool protocolRF::isDone()
{
	return m_rx_done;
}


//
// ----------------------------------------------------------------------------
/**
	   Routine: checkACK()
	   Inputs:  

	   Outputs:

    Check if CMD command was not received there ACK. retry if needed
 */
// ----------------------------------------------------------------------------
void protocolRF::checkACK()
{
	if(!mListACK.empty()) {
		std::list<protocolRF::ACKCmd_t>::iterator i;

		int iTime = getTime() ;

		for(i=mListACK.begin(); i != mListACK.end(); ++i)
		{
			if ( (iTime - i->Time) > TIME_OUT_ACK ||  i->Time > iTime )
			{
				YDLE_WARN << "Ack not receive from receptor: " << (int)i->Frame.receptor;
				// if more than 2 retry, then remove it
				if( i->iCount >=2)
				{
					YDLE_WARN << "ACK never received.";
					i=mListACK.erase(i);		// TODO : Send IHM this error
				}
				else
				{
					memcpy(&m_sendframe, &i->Frame,sizeof(Frame_t));
					m_sendframe.taille--; // remove CRC.It will be add in the transmit function
					i->Time=iTime;
					i->iCount++;
					transmit(true);	// re-Send frame;
				}		 
			}
		}
	}
}

int protocolRF::getTime()
{
	struct timeval localTime;
	gettimeofday(&localTime, NULL); 
	int iTime=localTime.tv_sec * 1000000;
	iTime+=localTime.tv_usec;
	return iTime;
}

void protocolRF::Start()
{
	// Start listen thread
	if (pthread_create(&_thread, NULL, protocolRF::listenSignal, this) < 0) {
  		throw std::runtime_error("Can't start ListenRF thread");

	}
	
}

void protocolRF::InitPlugin()
{
	SettingsParser * pSettings = SettingsParser::Instance() ;
	int rx_pin = pSettings->Int("rx_pin");
	int tx_pin = pSettings->Int("tx_pin");
	Init (rx_pin, tx_pin) ;

}
