/*
 * NodeRequestHandler.cpp
 *
 *  Created on: Dec 20, 2013
 *      Author: denia
 */
#include "logging.h"
#include "INode.h"
#include "NodeRequestHandler.h"
#include <cstring>
#include <algorithm>

using namespace ydle ;

namespace WebServer {

#define NBSEND 1

static struct timespec WaitEndCmommand={0,500000000L};

NodeRequestHandler::NodeRequestHandler(INodesManager* n) : _pNodes(n)
{
}

NodeRequestHandler::~NodeRequestHandler()
{
	// TODO Auto-generated destructor stub
}


int NodeRequestHandler::Run(const HTTPRequest *request, HTTPResponse *response) {
	response->SetContentType(HTTPServer::CONTENT_TYPE_JSON);
	Json::StyledWriter *writer = new Json::StyledWriter();
	Json::Value root;

	string url = request->Url();

	if(std::count(url.begin(), url.end(), '/') > 1){
		// Split the URL
		const char *pch = std::strtok((char*)url.data(), "/" );
		pch = std::strtok(NULL, "/" );
		url = pch;
	}

	if(url.compare("on") == 0){
		Manage3Param (request, root, CMD_ON) ;
	}else if(url.compare("off") == 0){
		Manage3Param (request, root, CMD_OFF) ;
	}else if(url.compare("link") == 0){
		Manage2Param (request, root, CMD_LINK) ;
	}else if(url.compare("reset") == 0){
		Manage2Param (request, root, CMD_RESET) ;
	}else if(url.compare("") != 0){
		SetResponse (root, NULL, "node information") ;
	}
#ifdef TESTS_
	else if(url.compare("pause") == 0){
		Manage2Param (request, root, CMD_PAUSE) ;
	}
	else if(url.compare("debug") == 0){
		Manage2Param (request, root, CMD_DEBUG) ;
	}
	else if(url.compare("askdata") == 0){
		Manage2Param (request, root, CMD_ASK_DATA) ;
	}
	else if(url.compare("timeout") == 0){
		Manage3Param (request, root, CMD_SETUP) ;
	}
#endif // TESTS_
	else{
		SetResponse (root, "ko", "unknown action") ;
	}
		
	response->Append(writer->write(root));
	return response->Send();
}

void NodeRequestHandler::SetResponse (Json::Value & root, const char * result, const char * message)
{
	if (result) root["result"] = result;
	if (message) root["message"] = message;
	;
}


void NodeRequestHandler::Manage3Param (const HTTPRequest *request, Json::Value& root, int cmd)
{
	string nid = request->GetParameter("nid");
	string target = request->GetParameter("target");
	string sender = request->GetParameter("sender");

	if(nid.length() == 0) {
		SetResponse (root, "ko", " parameter <nid> is missing") ;
	} else if (target.length() == 0) {
		SetResponse (root, "ko", " parameter <target> is missing") ;
	} else if (sender.length() == 0) {
		SetResponse (root, "ko", " parameter <sender> is missing") ;
	}else{
		int result;
		NodeGeneric3 (atoi(sender.c_str()), atoi(target.c_str()), atoi(nid.c_str()), cmd,  &result);
		SetResponse (root, "ok", NULL) ;
	}
}


void NodeRequestHandler::Manage2Param (const HTTPRequest *request, Json::Value & root, int param)
{
	string target = request->GetParameter("target");
	string sender = request->GetParameter("sender");

	if (target.length() == 0) {
		SetResponse (root, "ko", " parameter <target> is missing") ;
	} else if (sender.length() == 0) {
		SetResponse (root, "ko", " parameter <sender> is missing") ;
	}else{
		int result;
		NodeGeneric2 (atoi(sender.c_str()), atoi(target.c_str()), param, &result);
		SetResponse (root, "ok", NULL) ;
	}
}

int NodeRequestHandler::NodeGeneric3(int sender, int target, int id, int cmd, int* result)
{
	YDLE_DEBUG << "enter in NodeGeneric3 " << sender << "/" << target << "/" << id << "/" << cmd <<std::endl;

  	//send signal NBSEND time
	for (int i=0; i<NBSEND; i++)
    {
		_pNodes->SendCmd (target, sender, id, cmd);
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}

int NodeRequestHandler::NodeGeneric2(int sender, int target, int cmd, int* result)
{
	YDLE_DEBUG << "enter in NodeGeneric2 " << sender << " " << target << " " << cmd <<std::endl;

  	//send signal NBSEND time
	for (int i=0; i<NBSEND; i++)
    {
		_pNodes->SendCmd (target, sender, target, cmd);
		nanosleep(&WaitEndCmommand,NULL); // Delay 400 mili. this prevend lost frame if multi transmit
	}

	*result = 1;
	return 1;
}
} /* namespace WebServer */

