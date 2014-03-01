#ifndef Master_H
#define Master_H


//----------------------------------------------------------------------
//  Project: Ydle
//-------------------------------------------------------------------------
//  Author: 
//
//-------------------------------------------------------------------------
//
//  Copyright 2013
//-------------------------------------------------------------------------
#include "NodesManager.h"
#include "restLoggerDestination.h"
#include "SettingsParser.h"
#include "IhmCommunicationThread.h"
#include "NodeRequestHandler.h"
#include "logging.h"
#include <memory>
#include <string>

namespace ydle {

class Master
{
public:
	Master (int argc, char **argv)  ;
	void	InitPlugins () ;
	void	InitProtocols () ;
	void	StartComm() ;
	NodesManager * NodesMgr () {return &_nodesManager ; }
private:
	void InitLog () ;
	void InitRestLog();

private:
	StdErrorLogDestination *stderr_log ;
	restLoggerDestination *restLog ;
	SettingsParser *	_pSettings ;
	std::unique_ptr<IhmCommunicationThread> _pCom;
	Kernel				_kernel ;
	NodesManager		_nodesManager ;
} ;

} ; // namespace ydle

#endif // Master_H
