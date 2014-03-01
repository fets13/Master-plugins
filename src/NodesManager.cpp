#include "NodesManager.h"
#include "logging.h"

using namespace ydle ;
using namespace std ;

void NodesManager::Init (Kernel * k)
{
	memset (_nodes, 0, sizeof (_nodes)) ;

	// search for node affection in config file
try {
	SettingsParser * pSettings = SettingsParser::Instance() ;
	// get number of configured nodes
	int nbNodes = pSettings->Int("nodes.number") ;
	// parse each configured node
	for (int i = 0 ; i < nbNodes; i++) {
		std::stringstream ss;
		ss << "nodes.node" << i+1 ;

		string node = pSettings->Str(ss.str()) ;
		printf ("node %d : <%s>=<%s>\n", i+1, ss.str().c_str(), node.c_str()) ;
		string nomPlugin ;
		tNodesList listNodes ;
		// try to parse node plugin name and list of nodes related
		if (decoder (node, nomPlugin, listNodes)) {
			printf ("\t : node<%s>  nomPlugin<%s>  \n", node.c_str(), nomPlugin.c_str());
			// try to get node plugin pointer from its name
			INode * pNode = k->Node (nomPlugin) ;
			// if node exists
			if (pNode != NULL) {
				printf ("\t\t : ") ;
				// for each node listed, memorize its plugin
				for (tNodesList::iterator it = listNodes.begin(); it != listNodes.end(); it++) {
					int numNode = *it ;
					printf ("%d ", numNode) ;
					if (_nodes[numNode] != NULL) {
						YDLE_FATAL << "NodesManager::Init FAILED, node " << nomPlugin << " num:" 
							<< numNode << " already set !!!" ; 
					}
					else {
						_nodes[numNode] = pNode ;
					}
				}
				printf ("\n") ;
			}
			else {
				YDLE_FATAL << "NodesManager::Init FAILED, node " << nomPlugin << " unknown !!!" ; 
			}
		}

	}
#ifdef DBG_NODES_CONFIG
	for (int i = 0 ; i < NB_NODES; i++) {
		if (_nodes[i] != NULL) {
			printf ("i:%d node<%s>\n", i, _nodes[i]->Name().c_str());
		}
	}
#endif // DBG_NODES_CONFIG
	// memorize list of protocols
	pProtocols = &(k->Protocols()) ;
}
catch (const runtime_error & e)  {
	YDLE_FATAL << "NodesManager::Init FAILED !!! : " << e.what() ;
}
}
void NodesManager::decoder (string & s, tNodesList & l)
{
    std::string delim = ",";

    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        int node = atoi (s.substr(start, end - start).c_str()) ;
		l.push_back (node) ;
        start = end + delim.length();
        end = s.find(delim, start);
    }
	if (start != end) {
        int node = atoi (s.substr(start, end - start).c_str()) ;
		l.push_back (node) ;
	}

}
bool NodesManager::decoder (string & s, string & nom, tNodesList & l)
{
    std::string delim = "|";
    auto start = 0U;
    auto end = s.find(delim);
    if (end == std::string::npos) return false ;

	nom = s.substr(start, end - start) ;
	start = end + delim.length();
	string nodes = s.substr(start, end - start) ;
	decoder (nodes, l) ;

	return true ;

    std::cout << s.substr(start, end);
}
INode * NodesManager::GetNode (Frame_t * frame)
{
	printf ("NodesManager::GetNode from frame %d\n", frame->receptor) ;
	return GetNode (frame->receptor) ;
}

INode * NodesManager::GetNode (int nodeId)
{
	printf ("NodesManager::GetNode(%d) %d\n", nodeId, __LINE__) ;
	return _nodes[nodeId] ;
}

int	NodesManager::SendCmd (int target, int sender, int param, int cmd)
{
	INode * pNode = GetNode(target) ;
	if (pNode == NULL) {
		YDLE_FATAL << "NodesManager::SendCmd FAILED, node " << target << " unknown !!!"  ;
	}
	Frame_t frame ;
	frame.FormatCmd (target, sender, param, cmd) ;
	frame.Dump("Dump from NodesManager::SendMsg") ;
	// TODO : Cmd will be sent to all protocols
	// need to find a way to address directly the right one
	for( Kernel::ProtocolList::iterator it = pProtocols->begin(); it != pProtocols->end(); ++it) {
		IProtocol * protocol = *it ;
		protocol->SendMsg (frame) ;
		printf ("\tProtocol : %s:SendMsg \n", protocol->Name().c_str()) ;
	}
	return 1 ;
}

