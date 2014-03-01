/*
 * IhmRestHandler.h
 *
 *  Created on: Jan 6, 2014
 *      Author: denia
 */

#ifndef IHMRESTHANDLER_H_
#define IHMRESTHANDLER_H_

#include <map>
#include <list>
#include <vector>
#include <thread>

#include "INode.h"
#include "NodesManager.h"

namespace ydle {

class IhmCommunicationThread {
public:
	IhmCommunicationThread(std::string web_address, NodesManager * nodeMgr);
	virtual ~IhmCommunicationThread();
	void run();
	void start();
	void stop();
	int putFrame(Frame_t& frame);
	int extractData(Frame_t& frame, int index,int &itype,int &ivalue);
	void AddToListCmd(Frame_t * cmd) ;
private:
	bool running;
	std::thread *thread_t;
	std::string web_address;
	std::list<Frame_t> ListCmd;
	pthread_mutex_t listcmd_mutex ;
	NodesManager *	_nodesManager ;
};

} /* namespace ydle */

#endif /* IHMRESTHANDLER_H_ */
