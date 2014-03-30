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

void IhmCommunicationThread::ThreadBegin()
{
	SetPauseMs (1000) ;
	YDLE_INFO << "Start Communication thread";
}

void IhmCommunicationThread::ThreadAction()
{
	pthread_mutex_lock(&this->listcmd_mutex);
	int size = this->ListCmd.size();
	pthread_mutex_unlock(&this->listcmd_mutex);

	if(size <= 0) {
		Pause () ;
		return ;
	}
	

	for(int i = 0; i < size; i++){
		pthread_mutex_lock(&this->listcmd_mutex);
		Frame_t frame = this->ListCmd.front();
		this->ListCmd.pop_front();
		pthread_mutex_unlock(&this->listcmd_mutex);

		if(this->putFrame(frame) == 0){
			// Failed to send the data, so re-insert the frame in the waiting list
			pthread_mutex_lock(&this->listcmd_mutex);
			this->ListCmd.push_back(frame);
			pthread_mutex_unlock(&this->listcmd_mutex);
		}
	}
	Pause () ;

}

int IhmCommunicationThread::putFrame(Frame_t & frame)
{
	string  post_data;
	stringstream buf;

		char sztmp[255];
		sprintf(sztmp,"IHM:Data Hex: ");
		for (int a=0;a<frame.taille-1;a++)
			sprintf(sztmp,"%s 0x%02X",sztmp,frame.data[a]);
		YDLE_DEBUG << sztmp;

	int sender = (int)frame.sender;
	int index = 0;

	stringstream request;
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
		stringstream buf;
		buf << "sender=" << sender << "&type=" << data.type << "&data=" << data.val << "\r\n" ;
		browser.doPost(request.str(), buf.str());
		index++;
	}


	return 1;
}



