/*
 * IhmRestHandler.cpp
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#include "curl/curl.h"
#include <iostream>
#include <thread>
#include <string.h>

#include "logging.h"
#include "INode.h"
#include "IhmCommunicationThread.h"
#include "RestBrowser.h"

using namespace std ;
using namespace ydle ;


IhmCommunicationThread::IhmCommunicationThread(string address, NodesManager * nodeMgr) :
	web_address(address), _nodesManager(nodeMgr)
{
	listcmd_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
}

IhmCommunicationThread::~IhmCommunicationThread() {

}
// ----------------------------------------------------------------------------
/**
 Routine: AddToListCmd()
 Inputs: Frame_t cmd


 Outputs:

 Add a trame for IHM
 */
// ----------------------------------------------------------------------------
void IhmCommunicationThread::AddToListCmd(Frame_t * cmd) {
	/* Enter the critical section */

	Frame_t tmp;
	memcpy(&tmp, cmd, sizeof(Frame_t));

	pthread_mutex_lock(&listcmd_mutex);
	ListCmd.push_back(tmp);
	/*Leave the critical section */
	pthread_mutex_unlock(&listcmd_mutex);
}

void IhmCommunicationThread::run(){
	YDLE_INFO << "Start Communication thread";
	Frame_t frame;
	int size;
	this->running = true;

	while(this->running){
		pthread_mutex_lock(&this->listcmd_mutex);
		size = this->ListCmd.size();
		pthread_mutex_unlock(&this->listcmd_mutex);

		if(size > 0){
			for(int i = 0; i < size; i++){
				pthread_mutex_lock(&this->listcmd_mutex);
				frame = this->ListCmd.front();
				this->ListCmd.pop_front();
				pthread_mutex_unlock(&this->listcmd_mutex);

				if(this->putFrame(frame) == 0){
					// Failed to send the data, so re-insert the frame in the waiting list
					pthread_mutex_lock(&this->listcmd_mutex);
					this->ListCmd.push_back(frame);
					pthread_mutex_unlock(&this->listcmd_mutex);
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int IhmCommunicationThread::putFrame(Frame_t & frame)
{
	std::string  post_data;
	std::stringstream buf;

		char sztmp[255];
		sprintf(sztmp,"IHM:Data Hex: ");
		for (int a=0;a<frame.taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,frame.data[a]);
		YDLE_DEBUG << sztmp;

	int sender = (int)frame.sender;
	int index = 0;

	std::stringstream request;
	request << "/api/node/data";
	INode::tNodeDataList dataList ;
	INode * pNode = _nodesManager->GetNode (sender) ;
	if (pNode == NULL) {
		YDLE_DEBUG << "putFrame : node:" << sender << " UNKNOWN" << "\n";
		return 0 ;
	}
	int ret = pNode->GetData (&frame, dataList) ;
	printf ("%d IhmCommunicationThread::putFrame ret=%d  node<%s>\n", __LINE__, ret, pNode->Name().c_str()) ;
	for( INode::tNodeDataList::iterator it = dataList.begin(); it != dataList.end(); ++it) {
		INode::sNodeData & data = *it ;

	printf ("%d  value f:%g   \n", __LINE__, data.val) ;
		YDLE_DEBUG << "Data received : From "<< sender << " Type : "
			<< data.type << " Value : " << data.val << "\n";

		RestBrowser browser(this->web_address);
		std::stringstream buf;
		buf << "sender=" << sender << "&type=" << data.type << "&data=" << data.val << "\r\n" ;
		browser.doPost(request.str(), buf.str());
		index++;
	}


	return 1;
}
void IhmCommunicationThread::start(){
	thread_t = new std::thread(&IhmCommunicationThread::run, this);
}

void IhmCommunicationThread::stop(){
	this->running = false;
}
#if 0
/*
 * Extract the value from the frame
 * Yes, I know this function should not be here. Denia.
 * */
int IhmCommunicationThread::extractData(Frame_t & frame, int index,int &itype,int &ivalue)
{
	uint8_t* ptr;
	bool bifValueisNegativ=false;
	int iCurrentValueIndex=0;
	bool bEndOfData=false;
	int  iLenOfBuffer = 0;
	int  iModifType=0;
	int  iNbByteRest;

	iLenOfBuffer=(int)frame.taille;
	ptr=frame.data;

	if(iLenOfBuffer <2) // Min 1 byte of data with the 1 bytes CRC always present, else there is no data
		return -1;
	iNbByteRest= (int)frame.taille-1;
	while (!bEndOfData)
	{
		itype=(unsigned char)*ptr>>4;
		bifValueisNegativ=false;

		// This is a very ugly code :-( Must do something better
		if(frame.type==TYPE_CMD)
		{
			// Cmd type if always 12 bits signed value
			iModifType=DATA_DEGREEC;
		}
		else if(frame.type==TYPE_ETAT)
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
		case DATA_HUMIDITY:
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
			bEndOfData =true;
	}

	return 0;
}
#endif



