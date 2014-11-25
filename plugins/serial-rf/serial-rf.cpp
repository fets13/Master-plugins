#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>
#include <sys/time.h>


#include "serial-rf.h"
#include "crc.h"

#define TIMEOUT_ACK	500

using namespace std;
using namespace ydle;


SerialRF::SerialRF ()
{
	mWaitAck	= false ;
	mSomething2Send = false ;
	mTimeOutAck = 500 ;
}

bool SerialRF::ReadHeader ()
{
	puint8_t ptr = puint8_t(&mHeaderIn);
	bool ret = mRS.ReadBuf (ptr++, 1) ; // lecture caractere debut
	if (!ret) {
//FETS			Err ("ReadHeader.ReadBuf FAILED\n") ;
		return false ;
	}
	// check start byte
	if (mHeaderIn.begin != SERIAL_MSG_BEGIN) {
//FETS			Err ("ReadHeader FAILED begin car: '%c' begin<%#02x> := <%#02x>\n", char(mHeaderIn.begin), mHeaderIn.begin, SERIAL_MSG_BEGIN) ;
		return false ;
	}
	
	if( !mRS.ReadBuf (ptr++, 1)) { // lecture type
		Err ("ReadHeader.ReadBuf type FAILED\n") ;
		return false ;
	}
	// check type
	if (mHeaderIn.type < MSG_NORMAL || mHeaderIn.type >= MSG_COUNT) {
		Err ("ReadHeader FAILED : type<%d> >\n", mHeaderIn.type) ;
		return false ;
	}

	if( !mRS.ReadBuf (ptr++, 1)) { // lecture size
		Err ("ReadHeader.ReadBuf size FAILED\n") ;
		return false ;
	}
	// check size
	if (mHeaderIn.size > SERIAL_MSG_MAX) {
		Err ("ReadHeader FAILED : size<%d> > <%d>\n", mHeaderIn.size, SERIAL_MSG_MAX) ;
		return false ;
	}

	if( !mRS.ReadBuf (ptr++, 1)) { // lecture crc
		Err ("ReadHeader.ReadBuf crc FAILED\n") ;
		return false ;
	}
	return true ; // lecture crc
}

bool SerialRF::ReadMsg ()
{
	if (!ReadHeader ()) {
//FETS			Err ("ReadMsg.ReadHeader FAILED\n") ;
		return false ;
	}
	// it seems, we have a complete header
	if (mHeaderIn.size==0 && mHeaderIn.crc==0) {
		return true;
	}

	int remain = 0 ;
	int lus = 0 ;
	int sz = mHeaderIn.size ;
	puint8_t pData = mDataIn ;
	int nb = 0 ;
	// Attempt to read the remaining data
	while (!mRS.ReadBuf (pData, sz, &remain)) {
		usleep(30000);  /* sleep for 10 milliSeconds */
		if (remain == 0) {
			Err ("ReadMsg.ReadBuf(%d) FAILED : sz=%d remain=%d lus=%d\n", mHeaderIn.size, sz, remain, lus) ;
			return false ;
		}
		int lusLoc = (sz - remain) ;
		sz = remain ;
		pData += lusLoc ;
		lus += lusLoc ;
		if (++nb == 10) {
			Err ("ReadMsg.ReadBuf 2(%d) FAILED : sz=%d remain=%d lus=%d\n", mHeaderIn.size, sz, remain, lus) ;
			return false ;
		}
	}
#if 0
	if (!mRS.ReadBuf (mDataIn, mHeaderIn.size, &remain)) {
			Err ("ReadMsg.ReadBuf(%d) FAILED : remain=%d\n", mHeaderIn.size, remain) ;
			return false ;
	}
#endif
	// we have now the correct length
	// check if crc is good
	uint8_t crc = crc8(mDataIn, mHeaderIn.size) ;
	// check crc
	if (mHeaderIn.crc != crc) {
		Err ("ReadHeader FAILED : crc<%#02x> != <%#02x>\n", mHeaderIn.crc, crc) ;
		return false ;
	}
	return true;
}

bool SerialRF::WriteMsg (puint8_t buf, int size)
{
	if (mSomething2Send) {
		Err ("SerialRF::WriteMsg FAILED : message already in progress !!!\n") ;
		return false ;
	}
	// Prepare Header
	mHeaderOut.begin = SERIAL_MSG_BEGIN ;
	mHeaderOut.type = MSG_NORMAL ;
	mHeaderOut.size = size ;
	mHeaderOut.crc = crc8(buf, mHeaderOut.size) ;
	// Save daa to send
	memcpy (mDataOut, buf, size) ;
	mSomething2Send = true ;
	return true ;
}

bool SerialRF::Send()
{

	// send header
	bool ret = mRS.SendBuf (puint8_t(&mHeaderOut), sizeof(SerialHeader)) ;
	if (!ret) {
		Err ("SendBuf::Send.header(%d) FAILED : ret=%d\n", sizeof(SerialHeader), ret) ;
		return false ;
	}
	// send data
	ret = mRS.SendBuf (mDataOut, mHeaderOut.size) ;
	if (!ret) {
		Err ("SendBuf::Send(%d) FAILED : ret=%d\n", mHeaderOut.size, ret) ;
		return false ;
	}
	mWaitAck = true ;
	mTimeAck = GetTimeMs() ;
	return true;
}

bool SerialRF::SendAck(uint8_t status)
{
	SerialHeader header ;
	header.begin = SERIAL_MSG_BEGIN ;
	header.type = status ;
	header.size = 0 ;
	header.crc =  0 ;
#ifdef DBG_ACK
#define DBG_ACK
	puint8_t p = puint8_t(&header) ;
	printf ("ack:") ;
	for (uint32_t i = 0; i < sizeof(SerialHeader); i++) printf ("%#02x ", *p++) ;
	printf ("\n") ;
#endif // DBG_ACK
	bool ret = mRS.SendBuf (puint8_t(&header), sizeof(SerialHeader)) ;
	return ret ;
}

void SerialRF::ManageNewMsg (puint8_t msg, uint8_t sz)
{
	Frame_t * pFrame = (Frame_t *)msg ;
	pFrame->Dump ("SerialRF::ManageNewMsg") ;
	// If it's a RF ACK then handle it
	if (pFrame->type == TYPE_ACK) {
		if (mListMsg.CheckErase(pFrame->sender, pFrame->receptor)) {
			YDLE_DEBUG << "Remove ACK from pending list";
		}
	}
	// Node send data and whant ACK
	else if (pFrame->type == TYPE_ETAT_ACK) {
		YDLE_DEBUG << "SerialRF : New State/ACK frame ready to be sent :";
		// builds ACK frame
		mSendFrame.DataToFrame (pFrame->receptor,pFrame->sender, TYPE_ACK);				
		// Send ACK	
		WriteMsg (puint8_t(&mSendFrame), sizeof (mSendFrame)) ;
		// notify new frame to the master
		Notify (pFrame) ;
	}
	// Node send data
	else if (pFrame->type == TYPE_ETAT) {
		YDLE_DEBUG << "SerialRF : New State frame ready to be sent :";
		Notify (pFrame) ; // notiy new frame received
	}
	else {
		YDLE_DEBUG << "SerialRF : Bad frame, trash it :";
	}
		 
}

bool SerialRF::Loop()
{
//FETS		scheduler_realtime();

	while (1) {
		// if new message available
		if (ReadMsg ()) {
		// manage msg
			if (GetType () == MSG_ACK_OK) {
				// got ACK
				mWaitAck = false ;
				printf ("Got Serial ACK !!!\n") ;
			}
			// invalid ACK, need to send again
			else if (GetType () == MSG_ACK_NOK) {
				Send () ;
			}
			else {
				// here we have an actual message
				// send serial ack
				int ack = MSG_ACK_OK ;
				bool bRet = SendAck (ack) ;
				printf ("\tAck(%d) sent : ret=%d\n", ack, bRet) ;
				// we have to manage this new incoming msg
				printf ("Got new MSG type:%d size:%d\n", GetType(), GetSize());
				printf ("\tdata:") ;
				for (int i = 0; i < GetSize(); i++) {
					printf (" %#02x", GetData()[i]) ;
				}
				printf ("\n") ;
				ManageNewMsg (GetData(), GetSize());
			}
		}
		else {
			// no msg read from serial
			// do we have to do something (like sending serial msg)
			if (mSomething2Send) {
//FETS					printf ("%s:%d before Send\n", __FILE__, __LINE__) ;
				mSomething2Send = false ;
				Send () ;
//FETS					printf ("%s:%d after Send\n", __FILE__, __LINE__) ;
			}
		// if no msg, wait a little
			usleep(50000);  /* sleep for 50 milliSeconds */
			// if we exepected an ACK from arduino,
			// check if timeout elapsed
			if (mWaitAck) {
//FETS					printf ("%s:%d : mWaitAck=%d\n", __FILE__, __LINE__, mWaitAck) ;
				// if msg sent and no ack received yet
				if (GetTimeMs() - mTimeAck > mTimeOutAck) {
					printf ("Timeout send MSG again\n") ;
					Send () ;
				}
			}
		}
		// check if we need re-transmit	
		CheckACK();
	}
	return true ;
}

void* SerialRF::Loop(void* pParam)
{
	if (pParam == NULL) return NULL ;

//FETS		YDLE_DEBUG << "Enter in thread listen";
	printf ("SerialRF : Enter in thread \n");
	((SerialRF*)pParam)->Loop () ;

//FETS		YDLE_INFO << "Exit of thread listen";
	printf ("SerialRF : Exit thread \n");

	return NULL;
}


void SerialRF::Start()
{
	// Start listen thread
	if (pthread_create(&mThread, NULL, SerialRF::Loop, this) < 0) {
  		throw std::runtime_error("Can't start ListenSerial thread");

	}
}

void SerialRF::InitPlugin()
{
	SettingsParser * pSettings = SettingsParser::Instance() ;
	string device = pSettings->Str("serial-rf:device");
	int baudrate = pSettings->Int("serial-rf:baudrate");
	InitSerial (device.c_str(), baudrate) ;

}

void SerialRF::SendMsg (Frame_t & frame)
{
	memcpy (&mSendFrame, &frame, sizeof (Frame_t)) ;
	WriteMsg (puint8_t(&mSendFrame), sizeof (mSendFrame)) ;
	// if CMD, we wait for ack
	if (mSendFrame.type == TYPE_CMD) {
		mListMsg.AddNew (mSendFrame) ;
	}
}

bool SerialRF::InitSerial (PCSTR device, int baudrate)
{
	if(!mRS.Open (device, baudrate)) {
		Err("Serial::InitSerial FAILED Can not open comport <%s>\n", device);
		return false;
	}
	return true ;
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

void SerialRF::CheckACK()
{
	// if there is no pending ACK, bye-bye
	if(mListMsg.empty()) return ;

	MsgMemoList::iterator it;

	for(it = mListMsg.begin(); it != mListMsg.end(); ++it) {
		tMsgMemo * msg = *it ;
		if (msg->sent_ && msg->CheckTime (TIMEOUT_ACK)) {
			YDLE_WARN << "Ack not receive from receptor: " << (int)msg->frame_.receptor;
			// if more than 2 retry, then remove it
			if( msg->count_ >=2) {
				YDLE_WARN << "ACK never received.";
				mListMsg.Erase(it);		// TODO : Send IHM this error
			}
			else {
				// no ack received, need to send again 
				WriteMsg (puint8_t(&msg->frame_), sizeof (Frame_t)) ;
				msg->Update () ;
			}		 
		}
	}
}
