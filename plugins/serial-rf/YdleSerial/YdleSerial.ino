#include <SoftwareSerial.h>

/* Author : Fabrice Scheider AKA Denia
* Description : Sktech de test de la librairie
* Licence : CC-BY-SA
*/

#include <TimerOne.h>
#include "ydle.h"
#include "serial.h"
#include "common.h"

//FETS	static RS232 rs(5,4, 57600);
RS232 * rs ;
Stream*	mySerial ;

//FETS	static ydle y;
ydle * py;

unsigned long time = 0 ;
int count = 0 ;
void setup()
{
	py = new ydle ;
	ydle & y = *py ;
	Serial.begin(57600);
	rs = new RS232 (5,4, 9600) ;
	mySerial = rs->GetSoftSerial() ;
	y.init_timer();
	Serial.println("init complete");
count = 0 ;
time = 0 ;
        
}
uint8_t val = 1 ;
uint8_t tab[16] ;
void loop()
{
	ydle & y = *py ;
	Frame_t *frameRF = y.receive();
	// if there is an RF frame to manage : send it to serial
	if (frameRF != NULL) {
//FETS			// if RS not ready, wait
//FETS			if (!rs->IsReady2Send()) {
//FETS				y.CancelLastReceive() ;
//FETS			}
//FETS			else  {
		if (!rs->SendBuf (puint8_t(frameRF), sizeof(Frame_t))) {
			// failed to send cause not ready;
			y.CancelLastReceive() ;
			D1("RF msg received but serial not ready") ;
		}
	}
	// if a new Serial msg is received
	if (rs->Manager ()) {
		// check size : must be a Frame_t
		if (rs->GetMsgSize() != sizeof (Frame_t)) {
				D4 ("serial msg size received incorrect : ", rs->GetMsgSize(), " - ", sizeof(Frame_t)) ;
		}
		else {
			y.send ((Frame_t*)rs->GetDataIn()) ;
		}
	}
	/*
	if ((millis()-time) > 10000) {
		if (rs->IsReady2Send()) {
			
			for (int i = 0; i < sizeof(tab); i++) {
				tab[i] = val+i ;
			}
			val++;

			rs->SendBuf (tab, sizeof(tab)) ;
			time = millis();
		}
	}
	*/
}

