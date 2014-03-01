/*
 * NodeRequestHandler.h
 *
 *  Created on: Dec 20, 2013
 *      Author: denia
 */


#ifndef NODEREQUESTHANDLER_H_
#define NODEREQUESTHANDLER_H_
#include "INodesManager.h"
#include "webServer.h"
#include <jsoncpp/json/json.h>

namespace WebServer {

class NodeRequestHandler  : public HTTPServer::BaseHTTPCallback {
private:
	ydle::INodesManager *_pNodes;

public:
	NodeRequestHandler(ydle::INodesManager *p);
	virtual ~NodeRequestHandler();
    int Run(const HTTPRequest *request, HTTPResponse *response);
protected:
	void Manage3Param (const HTTPRequest *request, Json::Value &r, int cmd) ;
	void Manage2Param (const HTTPRequest *request, Json::Value &r, int cmd) ;
	int NodeGeneric2(int sender, int target, int cmd, int* result) ;
	int NodeGeneric3(int sender, int target, int id, int cmd, int* result) ;
	void	SetResponse (Json::Value &r, const char * result, const char * msg) ;
};

} /* namespace WebServer */

#endif /* NODEREQUESTHANDLER_H_ */
