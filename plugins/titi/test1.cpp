
#include "test1.h"

using namespace ydle ;
class NodeComplexe : public INode 
{
public:
	std::string Name () { return "EXAMPLE_NODE" ; }
	virtual void FormatCmd (int target, int sender, int param, int cmd) {};
	virtual int GetData (Frame_t *, tNodeDataList & list) {return 0 ;};
} ;


class ProtocolComplexe : public IProtocol 
{
public:
	virtual std::string Name () { return "EXAMPLE_PROTOCOL" ; }
	virtual void Start () {} ;
	virtual void debugMode(bool mode ) {} ;
	virtual void InitPlugin () {} ;
	void SendMsg (Frame_t & frame) {}
	
} ;

int	LoadPlugins (Kernel & k)
{
	k.RegisterNode (new NodeComplexe) ;
	k.RegisterProtocol (new ProtocolComplexe) ;

	return 1 ;
}
