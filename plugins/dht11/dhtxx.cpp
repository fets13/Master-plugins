
#include "dhtxx.h"
#include "DataAccess.h"

using namespace ydle ;
using namespace std ;


void NodeDhtXX::FormatCmd (int target, int sender, int param, int cmd)
{
}

int NodeDhtXX::GetData (Frame_t *frame, tNodeDataList & l)
{
	uint8_t * pData = frame->data ;
	sDht11Data * p = (sDht11Data *)pData ;
	printf ("NodeDhtXX[%s]::GetData : %#02x %#2x %#02x %#02x\n", Name().c_str()
			, pData[0] , pData[1] , pData[2] , pData[3]) ; 

	printf ("\t\t\t : f16  temp=%#0x  h=%#0x  [%d  %d]\n", p->temperature, p->humidity, p->temperature, p->humidity) ;
	sNodeData nodeData ;
	nodeData.type = DATA_DEGREEC ;
	nodeData.val = Float16To32(p->temperature) ;
	SetVal (frame->sender,  "temperature", nodeData.val) ;

	printf ("NodeDhtXX[%s]::GetData : temp=%g\n", Name().c_str(), nodeData.val) ;
	l.push_back (nodeData) ;
	nodeData.type = DATA_HUMIDITY ;
//FETS		nodeData.val = Float16To32(p->humidity) ;
	nodeData.val = (float)p->humidity ;
	SetVal (frame->sender,  "humidity", nodeData.val) ;
	printf ("NodeDhtXX[%s]::GetData : humidity=%g\n", Name().c_str(), nodeData.val) ;
	l.push_back (nodeData) ;

	return l.size() ;
}




int	LoadPlugins (Kernel & k)
{
//FETS		printf ("dht11::LoadPlugins  %d\n", __LINE__);
	k.RegisterNode (new NodeDhtXX) ;
	return 1 ;
}

