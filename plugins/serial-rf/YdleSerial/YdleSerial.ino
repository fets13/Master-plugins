#include <SoftwareSerial.h>

/* Author : Fets AKA Denia
* Description : Sktech de transfert de trames RF (resp serie) recues vers serie (resp RF)
* Licence : CC-BY-SA
*/

#include <TimerOne.h>
#include "ydle.h"
#include "serial.h"
#include "common.h"

RS232 * rs ;
Stream*	mySerial ;

ydle * py;

void setup()
{
	py = new ydle ;
	ydle & y = *py ;
	Serial.begin(57600);
	rs = new RS232 (5,4, 9600) ;
	mySerial = rs->GetSoftSerial() ;
	y.init_timer();
	Serial.println("init complete");
        
}
void loop()
{
	ydle & y = *py ;
	Frame_t *frameRF = y.receive();
	// if there is an RF frame to manage : send it to serial
	if (frameRF != NULL) {
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
}

