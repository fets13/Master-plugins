/**
 * Ydle - v0.5 (http://www.ydle.fr)
 * Home automotion project lowcost and DIY, all questions and information on http://www.ydle.fr
 *
 * @package Master
 * @license CC by SA
 **/



#include "master.h"
#include <wiringPi.h>

#include <signal.h>
#include <stdexcept>
#include <sstream>

#include "restLoggerDestination.h"
#include "webServer.h"
#include "schedul.h"

using namespace std;
using namespace ydle;


void exit_handler(int s) {
	YDLE_INFO << "Caught signal: " << s;
	scheduler_standard();
	exit(1);
}
// ----------------------------------------------------------------------------
/**
 Routine: main()
 Inputs:


 Outputs:

 Main entry program
 */
// ----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	Master master (argc, argv) ;


	if (setuid(0)) {
		perror("setuid");
		return 1;
	}

	// Config read ok, adjust the log level according to the user configuration

	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);


	//log("Program start");
	YDLE_DEBUG << "Program start";
	//End the program if the wiringPi library is not found
	if (wiringPiSetup() == -1) {
		YDLE_FATAL << "Error : Library wiringPi not found" << std::endl;
		return -1;
	}

	WebServer::HTTPServer::HTTPServerOptions options;
	int port = PARAM_INT("port");
	options.port = port;

	// Launch webserver
	WebServer::HTTPServer *server;
	server = new WebServer::HTTPServer(options);

	try {
		WebServer::NodeRequestHandler* n = new WebServer::NodeRequestHandler(master.NodesMgr());
		server->RegisterHandler("/node", n);
		server->Init();
		server->Run();

		master.StartComm() ;

		// TODO: (Denia) Temporaire
		while (1) {
			sleep(1000);
		}
	} catch (exception const& e) {
		YDLE_FATAL << "Something failed.  " << e.what() << endl;
	}

	YDLE_INFO << "program end"; // end execution .
	return 0 ;
}

Master::Master (int argc, char **argv) 
{
	InitLog () ;
	// Parse command line and config file
	_pSettings = SettingsParser::Instance() ;

	if(_pSettings->parseCommandLine(argc, argv) == 0){
      throw std::runtime_error("No parameters");
	}

	if(_pSettings->parseConfigFile() == 0){
		throw std::runtime_error("parse config file FAILED");
	}

	stderr_log->setLevel((ydle::log_level)PARAM_INT("logger.stderr.level"));
	InitRestLog();

	InitPlugins () ;
	InitProtocols () ;
	_nodesManager.Init (&_kernel) ;
}

void	Master::InitProtocols ()
{
	Kernel::ProtocolList & l = _kernel.Protocols() ;
printf ("Kernel : protocols=%u \n", l.size()) ;
	for( Kernel::ProtocolList::iterator it = l.begin(); it != l.end(); ++it) {
		IProtocol * rf = *it ;
		printf ("\tProtocol : %s \n", rf->Name().c_str()) ;
		rf->InitPlugin () ;
		// comment if you don't want debug
		rf->debugMode();
		rf->Start() ;
	}
}
void decoder (string & s)
{
    std::string delim = "|";

    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        std::cout << s.substr(start, end - start) << std::endl;
        start = end + delim.length();
        end = s.find(delim, start);
    }

    std::cout << s.substr(start, end);
}
void Master::StartComm()
{
	std::stringstream temp;
	temp << PARAM_STR("ihm_address") << ":" << PARAM_STR("ihm_port");
			
	_pCom = unique_ptr<IhmCommunicationThread>(new IhmCommunicationThread(temp.str(), NodesMgr()));
	Kernel::ProtocolList & l = _kernel.Protocols() ;
	for( Kernel::ProtocolList::iterator it = l.begin(); it != l.end(); ++it) {
		IProtocol * rf = *it ;
		printf ("\tProtocol : %s \n", rf->Name().c_str()) ;
		// subscrbe Ihm to protocols notifications
		rf->Subscribe (_pCom.get(), &IhmCommunicationThread::AddToListCmd) ;
	}
	_pCom->start();
}

void Master::InitLog () 
{
	// Init logging system
	stderr_log = new ydle::StdErrorLogDestination(ydle::YDLE_LOG_DEBUG);

	ydle::InitLogging(ydle::YDLE_LOG_DEBUG, stderr_log);
}


void Master::InitPlugins ()
{
	string dir = PARAM_STR("plugins.dir");
	YDLE_INFO << "Searching plugins into directory: " << dir;
	_kernel.LoadPlugins (dir) ;

	Kernel::NodeList & l = _kernel.Nodes() ;
printf ("Kernel : nodes=%u \n", l.size()) ;
	for( Kernel::NodeList::iterator it = l.begin(); it != l.end(); ++it) {
		INode * p = *it ;
		printf ("\tNode : %s \n", p->Name().c_str()) ;
	}
}

void Master::InitRestLog()
{
	std::stringstream ihm;
	ihm << PARAM_STR("ihm_address") << ":" << PARAM_STR("ihm_port");
	restLog = new ydle::restLoggerDestination(
					(ydle::log_level)PARAM_INT("logger.rest.level"),
					ihm.str()
			);
	restLog->Init();

	ydle::InitLogging(ydle::YDLE_LOG_DEBUG, restLog);
}

