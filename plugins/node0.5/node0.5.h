
#include "Kernel.h"

namespace ydle {

class Node0_5 : public INode 
{
public:
	std::string Name () { return "NODE VERSION 0.5" ; }
	virtual void FormatCmd (int target, int sender, int param, int cmd) {};
	virtual int GetData (Frame_t *, tNodeDataList & list) ;
private:
	bool ExtractData(uint8_t * & ptr, int &itype, float &fvalue) ;
} ;

} ;	 // namespace ydle
extern "C" int LoadPlugins (ydle::Kernel &) ;
