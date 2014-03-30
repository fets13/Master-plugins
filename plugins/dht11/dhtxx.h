
#include "Kernel.h"
#include "Float.h"

namespace ydle {

class NodeDhtXX : public INode 
{
public:
	typedef struct {
		float16	temperature ;
		int16_t	humidity ;
	} sDht11Data  ;
	NodeDhtXX () { ;}
	std::string Name () { return "DHT11" ; }
	virtual void FormatCmd (int target, int sender, int param, int cmd) ;
	virtual int GetData (Frame_t *, tNodeDataList & list) ;
} ;
} ;	 // namespace ydle
extern "C" int LoadPlugins (ydle::Kernel &) ;
