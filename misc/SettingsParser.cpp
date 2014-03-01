/*
 * Settings.cpp
 *
 *  Created on: Jan 11, 2014
 *      Author: denia
 */

#include "SettingsParser.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <cstdlib>
#include <libconfig.h++>
#include <getopt.h>
#include <stdexcept>

using namespace std;


namespace ydle {

SettingsParser * SettingsParser::_pInstance = NULL ;

SettingsParser::SettingsParser() : _pConfiguration(new libconfig::Config) {
}

SettingsParser * SettingsParser::Instance ()
{
	if (_pInstance == NULL) _pInstance = new SettingsParser ;
	return _pInstance ;
}

SettingsParser::~SettingsParser() {
}

int SettingsParser::parseSettings(){
	return 0;
}

void SettingsParser::showConfig(){
	int port, tx_pin, rx_pin;
	string listen_address;

	if(this->_pConfiguration->lookupValue("master.port", port)
		 && this->_pConfiguration->lookupValue("master.address", listen_address)
		 && this->_pConfiguration->lookupValue("master.rx_pin", rx_pin)
		 && this->_pConfiguration->lookupValue("master.tx_pin", tx_pin)){
		std::cout << "Master config " << std::endl;
		std::cout << "Port : " << port << std::endl;
		std::cout << "Listen address : " << listen_address << std::endl;
	} else {
		std::cout << "Config incomplete";
	}
}

int SettingsParser::parseCommandLine(int argc, char** argv) {
	int c;
	while ((c = getopt (argc, argv, "c:?::h::")) != -1){
		switch (c)
		{
		case 'h':
			this->showUsage();
			return 0;
		case 'c':
			config_file = optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
						"Unknown option character `\\x%x'.\n",
						optopt);
			return 0;
		default:
			break;
		}
	}
	if(config_file.length() <= 1){
		std::cout << "A parameter is missing, check your arguments" << std::endl;
		this->showUsage();
		return 0;
	}else{
		return 1;
	}
}
bool SettingsParser::configIsOk(){
	return false;
}

int SettingsParser::parseConfigFile(std::string config_file){
	try{
		this->_pConfiguration->readFile(config_file.c_str());
	}catch(libconfig::ParseException & pex){
		std::cout << "Error in config file : " << pex.what() << std::endl;
		std::cout << "at line: " << pex.getLine() << " " << pex.getError() << std::endl;
		return 0;
	}catch(std::exception & ex){
		std::cout << "Unable to read the config file : " << ex.what() << std::endl;
		return 0;
	}catch(...){
		std::cout << "Unknown exception" << std::endl;
		return 0;
	}
	return 1;
}

int SettingsParser::parseConfigFile(){
	return this->parseConfigFile(this->config_file);
}

void SettingsParser::showUsage(){
	std::cout << "Usage: ydlemaster -c [config file]" << std::endl;
	std::cout << "Example : ydlemaster -c ./ydle.conf" << std::endl;
	std::cout << "Options: "<< std::endl;
	std::cout << "Mandatory" << std::endl;
	std::cout << " -c : Path to config file" << std::endl;
	std::cout << " -h : Print this messsage" << std::endl;
}


int SettingsParser::Int (string name)
{
	return atoi(Str(name).c_str()) ;
}

string SettingsParser::Str (string name)
{
	string keyFull = "master." + name ;
	string val ;
	// if the key does not exist
	if(!_pConfiguration->lookupValue(keyFull, val)) {
		string strErr   ("key <" + name + "> does not exist !!!!") ;
		strErr = "key <" + name + "> does not exist !!!!" ;
		throw std::runtime_error(strErr);
	}
	return val ;
}

int SettingsParser::writeConfigFile(){
	return 1;
}

} /* namespace ydle */

