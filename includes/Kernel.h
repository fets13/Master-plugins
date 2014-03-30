#ifndef _Kernel_H_
#define _Kernel_H_

#include <list>
#include <map>
#include <string>
#include "IProtocol.h"
#include "INode.h"
#include "IFeature.h"
#include "DynLoad.h"
#include "ListPtr.h"

namespace ydle {
class Kernel ;

class Plugin
{
public:
	Plugin (std::string &) ;
	virtual ~Plugin ()  {};
	int		Register (Kernel &) ;
private:
	DynLoad mDyn ;
} ;


class Kernel
{
public:
    typedef std::map<std::string, Plugin*> PluginMap;
    typedef ListPtr<INode>		NodeList ;
    typedef ListPtr<IProtocol>	ProtocolList ;
    typedef ListPtr<IFeature>	FeatureList ;
public:
	virtual ~Kernel () ;

	void RegisterNode (INode * p) { this->_nodes.push_back (p ); }
	void RegisterProtocol (IProtocol * p) { this->_protocols.push_back (p ); }
	void RegisterFeature (IFeature * p) { this->_features.push_back (p ); }

	void	LoadPlugins (std::string & dir) ;

	PluginMap & 		Plugins () ;
	NodeList & 			Nodes () ;
	ProtocolList & 		Protocols () ;
	FeatureList & 		Features () ;
	INode *				Node (std::string name) ;
	IProtocol *			Protocol (std::string name) ;
	IFeature *			Feature (std::string name) ;
	
	typedef int	(*tPluginFonction) (Kernel & ) ;


private:
	NodeList		_nodes ;
	ProtocolList	_protocols ;
	FeatureList		_features ;

    /// <summary>Map of plugins by their associated file names</summary>
    private: PluginMap _loadedPlugins;
	
} ;

} ; // namespace ydle
#endif //  _Kernel_H_
