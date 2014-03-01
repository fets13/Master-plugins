
#include "dhtxx.h"
#include "Float.h"

using namespace ydle ;

class NodeDht11 : public INode 
{
public:
	typedef struct {
		float16	temperature ;
		int16_t	humidity ;
	} sDht11Data  ;
	NodeDht11 () { ;}
	std::string Name () { return "DHT11" ; }
	virtual void FormatCmd (int target, int sender, int param, int cmd) ;
	virtual int GetData (Frame_t *, tNodeDataList & list) ;
} ;

void NodeDht11::FormatCmd (int target, int sender, int param, int cmd)
{
}

int NodeDht11::GetData (Frame_t *frame, tNodeDataList & l)
{
	uint8_t * pData = frame->data ;
	sDht11Data * p = (sDht11Data *)pData ;
	printf ("NodeDht11[%s]::GetData : %#02x %#2x %#02x %#02x\n", Name().c_str()
			, pData[0] , pData[1] , pData[2] , pData[3]) ; 

	printf ("\t\t\t : f16  temp=%#0x  h=%#0x  [%d  %d]\n", p->temperature, p->humidity, p->temperature, p->humidity) ;
	sNodeData nodeData ;
	nodeData.type = DATA_DEGREEC ;
	nodeData.val = Float16To32(p->temperature) ;
	printf ("NodeDht11[%s]::GetData : temp=%g\n", Name().c_str(), nodeData.val) ;
	l.push_back (nodeData) ;
	nodeData.type = DATA_HUMIDITY ;
//FETS		nodeData.val = Float16To32(p->humidity) ;
	nodeData.val = (float)p->humidity ;
	printf ("NodeDht11[%s]::GetData : humidity=%g\n", Name().c_str(), nodeData.val) ;
	l.push_back (nodeData) ;

	return l.size() ;
}




int	LoadPlugins (Kernel & k)
{
	k.RegisterNode (new NodeDht11) ;
	return 1 ;
}

