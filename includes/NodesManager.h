
#ifndef NodeManager_H
#define NodeManager_H

#include "Kernel.h"
#include "INodesManager.h"
#include "IProtocol.h"
#include <list>
#include <string>

namespace ydle {

class NodesManager : public INodesManager
{
public:
	typedef std::list<int> tNodesList ;
public:
static const int NB_NODES = 256 ;
	NodesManager () {}; 
	void Init (Kernel * k) ;
	virtual int	SendCmd (int target, int sender, int param, int cmd) ;
	INode *	GetNode (Frame_t *) ;
	INode *	GetNode (int) ;
private:
	void decoder (std::string & s, tNodesList & l) ;
	bool decoder (std::string & s, std::string & nom, tNodesList & l) ;
	typedef INode * tpNode ;
	tpNode	_nodes[NB_NODES+1] ; // 2^8 maximum nodes
	Kernel::ProtocolList	* pProtocols ;
} ;

} ; // namespace ydle

#endif // NodeManager_H

