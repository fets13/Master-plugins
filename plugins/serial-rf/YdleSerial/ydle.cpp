// Ydle.cpp
//
// Ydle implementation for Arduino
// See the README file in this directory for documentation
// For changes, look at Ydle.h
//
// Authors:
// Fabrice Scheider AKA Denia,
// Manuel Esteban AKA Yaug
// Matthieu Desgardin AKA Zescientist
// Yargol AKA Yargol
//
// WebPage: http://www.ydle.fr/index.php
// Contact: http://forum.ydle.fr/index.php
// Licence: CC by sa (http://creativecommons.org/licenses/by-sa/3.0/fr/)
// Pll function inspired on VirtualWire library


#include <TimerOne.h>
#include "ydle.h"
//FETS  #include "Float.h"
#include <avr/eeprom.h>
//FETS	#include <Wire.h>
#include "serial.h"
#include "common.h"



const PROGMEM uint8_t _atm_crc8_table[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};


static Frame_t g_m_receivedframe;  // received frame
static Frame_t g_frameBuffer[YDLE_MAX_FRAME];
#ifdef TODO
static Frame_t g_sendFrameBuffer; // Une seule pour le moment
#endif TODO
//FETS	static Frame_t i2c_frame; // Frame utilisée dans le cas de l'I2C

static uint8_t m_data[YDLE_MAX_SIZE_FRAME]; // data + crc

static uint8_t pinRx = 12;		// Le numéro de la broche IO utilisée pour le module récepteur
static uint8_t pinTx = 10;		// Le numéro de la broche IO utilisée pour le module émetteur
static uint8_t pinLed = 13;		// Le numéro de la broche IO utilisée pour la Led de statut
#ifdef USE_RECEIVER_OFF
static uint8_t pinReceiverOff = 6;		// Le numéro de la broche IO utilisée pour couper l'alim du recepteur RF pour le pas le perturber durant l'émisson
#endif // USE_RECEIVER_OFF

static uint8_t start_bit2 = 0b01000010; // Octet de start

volatile uint8_t sample_value = 0;		// Disponibilité d'un sample
volatile uint8_t sample_count = 1;		// Nombre de samples sur la période en cours
volatile uint8_t last_sample_value = 0; // La valeur du dernier sample reéu

// La rampe PLL, varie entre 0 et 159 sur les 8 samples de chaque période de bit
// Quand la PLL est synchronisée, la transition de bit arrive quand la rampe vaut 0
static uint8_t pll_ramp = 0;

// La somme des valeurs des samples. si inférieur à 5 "1" samples dans le cycle de PLL
// le bit est déclaré comme 0, sinon à 1
static uint8_t sample_sum = 0;
static uint16_t rx_bits = 0;		// Les 16 derniers bits reçus, pour repérer l'octet de start
static uint8_t rx_active = 0;		// Flag pour indiquer la bonne réception du message de start


#define YDLE_SPEED 1000					// Le débit de transfert en bits/secondes
#define YDLE_TPER 1000000/YDLE_SPEED	// La période d'un bit en microseconds
#define YDLE_FBIT YDLE_TPER/8			// La fréquence de prise de samples
//FETS	#define INPROCESS 7						// Le pin pour signaler une trame à envoyer du RPI
//FETS	#define GIFTFORYOU 6					// Le pin pour signaler au RPI qu'on a reçu une trame
//FETS	#define THIS_ADDRESS 0x9				// L'adresse I2C du 328p

static uint8_t bit_value = 0;	// La valeur du dernier bit récupéré
static uint8_t bit_count = 0;	// Le nombre de bits récupérés
static uint8_t sender = 0;		// Id sender reçue
static uint8_t receptor = 0;	// Id receptor reçue
static uint8_t type = 0;		// Info type reçue
static uint8_t taille = 0;		// Info taille reçue
static int rx_bytes_count = 0;	// Nombre d'octets reçus
static uint8_t length_ok = 0;	// Disponibilité de la taille de trame
static uint8_t frameBufferNumber = 0;
//FETS	static uint8_t I2C_send = false;

volatile uint8_t wait_ack = 0;
volatile uint8_t last_check = 0;
volatile uint8_t retry = 0;

static uint8_t frameReadyToBeRead = false;
//FETS	static uint8_t frameReadyToBeSend = false;
static uint32_t gCptFrameIn = 0;
volatile uint8_t transmission_on = false;

static int tx_sample =0;
static int tx_bit =7;
static int tx_index =0;
static bool bit_test = false;
volatile uint8_t frameToSend[40];
volatile uint8_t frameToSendLength = 0;


// Initialisation des IO avec les valeurs par défaut, en mode I2C
ydle::ydle()
{
	pinMode(pinRx, INPUT);
	pinMode(pinTx, OUTPUT);
	pinMode(pinLed, OUTPUT);
#ifdef USE_RECEIVER_OFF
	pinMode(pinReceiverOff, OUTPUT);
	digitalWrite(pinReceiverOff, HIGH);
#endif // USE_RECEIVER_OFF

#if 0
	pinMode(INPROCESS, OUTPUT);
	digitalWrite(INPROCESS, LOW);
    pinMode(GIFTFORYOU, OUTPUT);
    digitalWrite(GIFTFORYOU, LOW);
    Wire.begin(THIS_ADDRESS);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
#endif
	m_Config.IdMaster = 1 ;
	mReadFrame = 0 ;
}

void ydle::init_timer(){
	Timer1.initialize(YDLE_FBIT); // set a timer of length YDLE_FBIT microseconds
	Timer1.attachInterrupt( timerInterrupt ); // attach the service routine here
}

void timerInterrupt(){
	if(!transmission_on){
		sample_value = digitalRead(pinRx);
		pll();
	}
		// if sending in progress
	if(transmission_on){
		// send one bit every 8 timerInterrupt cycles
		if(tx_sample == 0){
			if (bit_test ==0){
				digitalWrite(pinTx, frameToSend[tx_index]& 1<<tx_bit);
				bit_test = true;
			}
			else {
				digitalWrite(pinTx, !(frameToSend[tx_index]& 1<<tx_bit));
				bit_test = false;
				tx_bit--; // next bit of sample
				// if last bit (0), we restart to 8th of next sample
				if (tx_bit < 0){
					tx_index++;
					tx_bit = 7;
				}
			}

		}
		tx_sample++;
		if(tx_sample >= 8) {
			tx_sample = 0 ;
		}
		// if last bit of sample sent
		if(tx_index > frameToSendLength && tx_sample == 0){
			transmission_on=false;	// sending is over
			digitalWrite(pinTx, LOW);
			digitalWrite(pinLed, LOW);
#ifdef USE_RECEIVER_OFF
			digitalWrite(pinReceiverOff, HIGH);
#endif // USE_RECEIVER_OFF
//FETS				digitalWrite(INPROCESS, LOW);
		}
	}
}

uint8_t ydle::crc8(const uint8_t* buf, uint8_t length) {
	// The inital and final constants as used in the ATM HEC.
	const uint8_t initial = 0x00;
	const uint8_t final = 0x55;
	uint8_t crc = initial;
	while (length) {
		crc = pgm_read_byte_near(_atm_crc8_table + (*buf ^ crc));
		buf++;
		length--;
	}
	return crc ^ final;
}

uint8_t ydle::computeCrc(Frame_t* frame){
	uint8_t *buf, crc;
	int a,j;

	buf = (uint8_t*)malloc(frame->taille+3);
	memset(buf, 0x0, frame->taille+3);

	buf[0] = frame->sender;
	buf[1] = frame->receptor;
	buf[2] = frame->type;
	buf[2] = buf[2] << 5;
	buf[2] |= frame->taille;

	for(a=3, j=0 ;j < frame->taille - 1;a++, j++){
		buf[a] = frame->data[j];
	}

	crc = crc8(buf,frame->taille+2);
	free(buf);
	return crc;
}

void	ydle::CancelLastReceive()
{
	if (mReadFrame > 0) mReadFrame--;
}


Frame_t * ydle::receive()
{
#if 0
	if (frameReadyToBeSend == true){
		frameReadyToBeSend = false;
		send(&i2c_frame);
	}
#endif
	if (mReadFrame >= gCptFrameIn) return NULL ;

	Frame_t * frame = &g_frameBuffer[mReadFrame%YDLE_MAX_FRAME] ;
	mReadFrame++ ;
	D4("S:",frame->sender, " R:",frame->receptor);

	uint8_t crc_p = computeCrc(frame);
	if (crc_p != frame->crc) {
#ifdef _YDLE_DEBUG
		printFrame(frame);
		log("crc error!!!!!!!!!");
#endif // _YDLE_DEBUG
		return NULL ;
	}
	return frame ;
#if 0
	uint8_t crc_p;
	
	// at least one frame to manage
	if(frameReadyToBeRead){
		for(uint8_t i = 0; i < gCptFrameIn; i++){
			crc_p = computeCrc(&g_frameBuffer[i]);
			if(crc_p != g_frameBuffer[i].crc)
			{
#ifdef _YDLE_DEBUG
				printFrame(&g_frameBuffer[i]);
				log("crc error!!!!!!!!!");
#endif // _YDLE_DEBUG
			}
			else
			{
#ifdef _YDLE_DEBUG
				Serial.println("Frame ready to be handled");
#endif // _YDLE_DEBUG
				if(g_frameBuffer[i].type == YDLE_TYPE_ACK){
					Serial.println("ACK received");
					if(g_frameBuffer[i].sender == g_sendFrameBuffer.receptor 
						&& g_frameBuffer[i].receptor == g_sendFrameBuffer.sender){
						wait_ack = 0;
						retry = 0;
						last_check = 0;
						// nothing more to manage here
					}
				}
				else {
#ifdef _YDLE_DEBUG
					printFrame(&g_frameBuffer[frameBufferNumber]);
#endif
//FETS						if (g_frameBuffer[i].type == YDLE_TYPE_STATE_ACK) {
//FETS							Serial.println ("YDLE_TYPE_STATE_ACK") ;
//FETS							handleReceivedFrame(&g_frameBuffer[i]);
//FETS						}
					// TODO : envoi PI
					
#if 0
					/*for (int k = 0; k < 30; k++){
						Serial.print(trameToSend[k]);
						Serial.print(" - ");
					}
					Serial.println(" ");*/
					digitalWrite(GIFTFORYOU, HIGH);
					delayMicroseconds(100);
					
					// TODO Zescientist, code horrible à changer
					while(I2C_send != true){
						delayMicroseconds(100);
					}
					digitalWrite(GIFTFORYOU, LOW);
					delayMicroseconds(100);
#endif
//FETS						Timer1.initialize(YDLE_FBIT);
//FETS						Timer1.attachInterrupt(timerInterrupt);
				}			
			}
		}
		// Peut poser un probléme si une interruption se produit et
		// que la pll termine de traiter un paquet et la pose sur la pile exactement à ce moment là
		gCptFrameIn = 0;
		frameReadyToBeRead = false;
		if(wait_ack == 1){
			if(retry <= 3){
				uint8_t curt = millis();
				if(curt - last_check >= YDLE_ACK_TIMEOUT){
					last_check = curt;
					this->send(&g_sendFrameBuffer);
#ifdef _YDLE_DEBUG
					Serial.println("Timeout, resending frame");
#endif
				}
				retry++;
			}else{
				// Lost packet... sorry dude !
				wait_ack = 0;
				retry = 0;
				last_check = 0;
#ifdef _YDLE_DEBUG
				Serial.println("Lost packet... sorry dude !");
#endif
			}
		}
	}
#endif
}	
	
// Do something with a received Command
void ydle::handleReceivedFrame(Frame_t *frame)
{
Serial.println ("handleReceivedFrame 1") ;

	// send ACK if frame is for us.
	if(checkSignal(frame))
	{
  Serial.println ("handleReceivedFrame before send_ack") ;

		delay(200);
		#ifdef _YDLE_DEBUG
		log("**************Send ACK********************");
		#endif
		Frame_t response;
		memset(&response, 0x0, sizeof(response));
		dataToFrame(&response, YDLE_TYPE_ACK, frame->sender);	// Create a new ACK Frame
		#ifdef _YDLE_DEBUG
		printFrame(&response);
		Serial.println("End send ack");
		#endif
		send(&response);
	}
}

// Synchronise l'AGC, envoie l'octet de start puis transmet la trame
void ydle::send(Frame_t *frame)
{
//FETS		digitalWrite(INPROCESS, HIGH);

	digitalWrite(pinLed, HIGH);   // on allume la Led pour indiquer une émission

	// add crc BYTE
	frame->taille++;
	// calcul crc
	frame->crc = computeCrc(frame);

#ifdef TODO
	if(frame->type == YDLE_TYPE_STATE_ACK){
		if(wait_ack != 1){
			memcpy(&g_sendFrameBuffer, &frame, sizeof(Frame_t));
			wait_ack = 1;
		}
	}
#endif TODO

#ifdef _YDLE_DEBUG
	Serial.println("envois");
	printFrame(frame);
#endif
	// From now, we are ready to transmit the frame

	while(rx_active){
		// Wait that the current transmission finish
		delay(2*YDLE_TPER);
#ifdef _YDLE_DEBUG
		Serial.println("The ligne is occuped");
#endif
	}
	memset((void*)&frameToSend, 0x0, 40);

	uint8_t index =0;

	frameToSendLength =7+frame->taille;
	frameToSend[index++] = 0xFF;
	frameToSend[index++] = 0xFF;
	frameToSend[index++] = 0xFF;
	frameToSend[index++] = 0xFF;
	frameToSend[index++] = start_bit2;
	frameToSend[index++] = frame->receptor;
	frameToSend[index++] = frame->sender;
	frameToSend[index++] = (frame->type << 5)+ frame->taille;
	for (int j=0; j<frame->taille-1;j++)
	{
		frameToSend[index++] = frame->data[j];
	}
	frameToSend[index++] = frame->crc;

	tx_index = 0;	// sample iterator
	tx_bit = 7;		// bit iterator

#ifdef USE_RECEIVER_OFF
	digitalWrite(pinReceiverOff, LOW);	 // used to shutdown receiver 
#endif // USE_RECEIVER_OFF
	// we want to send a frame
	transmission_on = true;
}

// Comparaison du signal reçu et du signal de référence
bool ydle::checkSignal(Frame_t *frame)
{
  Serial.print ("receptor:"); Serial.print (frame->receptor);
  Serial.print ("  master:"); Serial.println (m_Config.IdMaster);
	if(frame->receptor==m_Config.IdMaster)
		return true;
 	else
		return false;
}


void pll()
{
	sample_count ++;
	// On additionne chaque sample et on incrémente le nombre du prochain sample
	if (sample_value){
		sample_sum++;
	}
	
	// On vérifie s'il y a eu une transition de bit
	if (sample_value != last_sample_value){
		// Transition, en avance si la rampe > 40, en retard si < 40
		if(pll_ramp < 80){
			pll_ramp += 11;
		} 
		else{
			pll_ramp += 29;
		}
		last_sample_value = sample_value;
	}
	else{
		// Si pas de transition, on avance la rampe de 20 (= 80/4 samples)
		pll_ramp += 20;
	}
	
	// On vérifie si la rampe é atteint son maximum de 80
	if (pll_ramp >= 160)
	{
		// On ajoute aux 16 derniers bits reéus rx_bits, MSB first
		// On stock les 16 derniers bits
		rx_bits <<= 1;
		
		// On vérifie la somme des samples sur la période pour savoir combien était é l'état haut
		// S'ils étaient < 2, on déclare un 0, sinon un 1;
		if (sample_sum >= 5){
			rx_bits |= 0x1;
			bit_value = 1;
		}
		else{
			rx_bits |= 0x0;
			bit_value = 0;
		}
		pll_ramp -= 160; // On soustrait la taille maximale de la rampe é sa valeur actuelle
		sample_sum = 0; // On remet la somme des samples é 0 pour le prochain cycle
		sample_count = 1; // On ré-initialise le nombre de sample

		// Si l'on est dans le message, c'est ici qu'on traite les données
		if (rx_active){
			bit_count ++;
			// On récupére les bits et on les places dans des variables
			// 1 bit sur 2 avec Manchester
			if (bit_count % 2 == 1){
				if (bit_count < 16){
					// Les 8 premiers bits de données
					receptor <<= 1;
					receptor |= bit_value;
				}
				else if (bit_count < 32){
					// Les 8 bits suivants
					sender <<= 1;
					sender |= bit_value;
				}
				else if (bit_count < 38){
					// Les 3 bits de type
					type <<= 1;
					type |= bit_value;
				}
				else if (bit_count < 48){
					// Les 5 bits de longueur de trame
					rx_bytes_count <<= 1;
					rx_bytes_count |= bit_value;
				}
				else if ((bit_count-48) < (rx_bytes_count * 16)){
					// les données
					length_ok = 1;
					m_data[(bit_count-48)/16] <<= 1;
					m_data[(bit_count-48)/16]|= bit_value;
				}
			}
			
			// Quand on a reçu les 24 premiers bits, on connait la longueur de la trame
			// On vérifie alors que la longueur semble logique
			if (bit_count >= 48)
			{
				// Les bits 19 é 24 informent de la taille de la trame
				// On les vérifie car leur valeur ne peuvent étre < é 1 et > é 30 + 1 pour le CRC
				if (rx_bytes_count < 1 || rx_bytes_count > 30)
				{
#ifdef _YDLE_DEBUG
					Serial.println("error!");
#endif
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
//FETS						digitalWrite(INPROCESS, LOW);
					return;
				}
			}

			// On vérifie si l'on a reçu tout le message
			if ((bit_count-48) >= (rx_bytes_count*16) && (length_ok == 1))
			{
#ifdef _YDLE_DEBUG
				Serial.println("complete");
#endif

				rx_active = false;
				g_m_receivedframe.sender = sender;
				g_m_receivedframe.receptor = receptor;
				g_m_receivedframe.type = type;
				g_m_receivedframe.taille = rx_bytes_count; // data + crc
				memcpy(g_m_receivedframe.data,m_data,rx_bytes_count-1); // copy data len - crc
				g_m_receivedframe.crc=m_data[rx_bytes_count-1];

				// May be an array ?
//FETS					if(gCptFrameIn == YDLE_MAX_FRAME){
//FETS						gCptFrameIn = 0;
//FETS					}
//FETS					memcpy(&g_frameBuffer[gCptFrameIn], &g_m_receivedframe, sizeof(Frame_t));
				memcpy(&g_frameBuffer[gCptFrameIn%YDLE_MAX_FRAME], &g_m_receivedframe, sizeof(Frame_t));
				gCptFrameIn++;
				frameReadyToBeRead = true;

				length_ok = 0;
				sender = 0;
				receptor = 0;
				type = 0;
				taille = 0;
				memset(m_data,0,sizeof(m_data));
//FETS					digitalWrite(INPROCESS, LOW);
			}

		}

		// Pas dans le message, on recherche l'octet de start
		else if (rx_bits == 0x6559)
		{
#ifdef _YDLE_DEBUG
			Serial.println("start");
#endif
			// Octet de start, on commence é collecter les données
			rx_active = true;
			bit_count = 0;
			rx_bytes_count = 0;
//FETS				digitalWrite(INPROCESS, HIGH);
		}
	}
}

// Fonction qui crée une trame avec un type fournie
void ydle::dataToFrame(Frame_t *frame, unsigned long type, uint8_t this_receptor)
{
	frame->sender = m_Config.IdMaster;
	frame->receptor = this_receptor;
	frame->type = type;
	frame->taille = 0;
	frame->crc = 0;
	memset(frame->data, 0x0, sizeof(frame->data));
}

int ydle::isSignal()
{
	return rx_active;
}


// Affiche les logs sur la console série
void ydle::log(String msg)
{
#if not defined( __AVR_ATtiny85__ ) or defined(_YDLE_DEBUG)
	Serial.println(msg);
#endif
}
// Affiche les logs sur la console série
void ydle::log(String msg,int i)
{
#if not defined( __AVR_ATtiny85__ ) or defined(_YDLE_DEBUG)
	Serial.print(msg);
	Serial.println(i);
#endif
}

// ----------------------------------------------------------------------------
/**
	   Function: printFrame
	   Inputs:  Frame_t trame  frame to log
				int data

	   Outputs: 
		Log a frame if debug activated
*/
// ----------------------------------------------------------------------------
void ydle::printFrame(Frame_t *trame)
{
#if not defined( __AVR_ATtiny85__ ) or defined (_YDLE_DEBUG)
	// if debug
		char sztmp[255];
		
		D1("-----------------------------------------------");
		sprintf(sztmp,"E:%d R:%d T:%d Sz:%d CRC:%0#x",trame->sender,trame->receptor,trame->type,trame->taille,trame->crc);
		D1(sztmp);
		sprintf(sztmp,"Data Hex: ");
		for (int a=0;a<trame->taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,trame->data[a]);
		D1(sztmp);
#if 0
		sprintf(sztmp,"Emetteur :%d",trame->sender);
		log(sztmp);

		sprintf(sztmp,"Recepteur :%d",trame->receptor);
		log(sztmp);
		
		sprintf(sztmp,"Type :%d",trame->type);
		log(sztmp);

		sprintf(sztmp,"Taille :%d",trame->taille);
		log(sztmp);

		sprintf(sztmp,"CRC :%d",trame->crc);
		log(sztmp);

		sprintf(sztmp,"Data Hex: ");
		for (int a=0;a<trame->taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,trame->data[a]);
		log(sztmp);

		sprintf(sztmp,"Data Dec: ");
		for (int a=0;a<trame->taille-1;a++)
			sprintf(sztmp,"%s %d",sztmp,trame->data[a]);
		log(sztmp);
#endif
		D1("-----------------------------------------------");
#endif
}

#if 0
void ydle::requestEvent() {
	byte trameToSend[30];
	memset(&trameToSend, 0x0, sizeof(trameToSend));
	memcpy(trameToSend, &g_frameBuffer[frameBufferNumber], (g_frameBuffer[frameBufferNumber].taille)+3);
	Wire.write(trameToSend, 30); //,(g_frameBuffer[frameBufferNumber].taille)-1
	I2C_send = true;
}

void ydle::receiveEvent(int howMany) {
	int i = 0;
	int j = 0;
	byte t_register = 0;
    byte trame[30];
    memset(&trame, 0x0, sizeof(trame));
   	while(Wire.available())
    {
    	if(j == 0){
       		// on zappe le premier octet
			t_register = Wire.read();
			j++;
		} else {
			trame[i] = Wire.read();
			i++;
		}
    }    
    if(t_register == 0x0){
    	return;
    } else if (t_register == 0x01) {
    	memcpy(&i2c_frame, trame, (trame[4]&= 0x1F));
    	frameReadyToBeSend = true;
    }
}

#endif
