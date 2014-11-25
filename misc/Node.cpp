
#include "INode.h"
#include "logging.h"
#include "crc.h"
#include <string.h> // memset

using namespace ydle ;

void Frame_t::DataToFrame(uint8_t transmitter, uint8_t receiver, uint8_t type_)
{
	memset (data, 0, sizeof(data));
	sender = transmitter;
	receptor = receiver;
	type = type_;
	taille = 0;
	crc = 0;
} 

void Frame_t::AddCmd(uint8_t typeIn, uint8_t dataIn)
{
	data[taille] = typeIn<<4;
	data[taille+1] = dataIn;
	taille += 2;
}

void Frame_t::FormatCmd (uint8_t target, uint8_t sender, uint8_t param, uint8_t cmd)
{
	DataToFrame (sender, target, TYPE_CMD) ;
	AddCmd (cmd, param) ;
}

#if 1

// ----------------------------------------------------------------------------
/*
	   Routine: printFrame
	   Inputs:  


	   // log Frame
 */
// ----------------------------------------------------------------------------

void Frame_t::Dump (const char * msg)
{
	char sztmp[255];
	if (msg) YDLE_DEBUG << "\t" << msg ;
	YDLE_DEBUG << "Sender:" << (int)sender
				<< " Receptor:" << (int)receptor
				<< " Type:" << (int)type
				<< " Size:" << (int)taille
				<< " CRC:" << std::hex << (int)crc;


	sprintf(sztmp,"Data Hex: ");
	for (int a=0;a<taille-1;a++)
		sprintf(sztmp,"%s 0x%02X",sztmp,data[a]);
	YDLE_DEBUG << sztmp;

	sprintf(sztmp,"Data Dec: ");
	for (int a=0;a<taille-1;a++)
		sprintf(sztmp,"%s %d",sztmp,data[a]);
	YDLE_DEBUG << sztmp;
}

#endif
