#ifndef _Plugin_H_
#define _Plugin_H_

#include <list>
#include <map>
#include <string>
#include "IProtocol.h"
#include "INode.h"
#include "DynLoad.h"

namespace ydle {
class Kernel ;

class Plugin
{
public:
	Plugin (std::string &) ;
	bool	Register (Kernel &) ;
	virtual ~Plugin ()  {};
private:
	DynLoad mDyn ;
} ;


class Kernel
{
public:
    typedef std::map<std::string, Plugin*> PluginMap;
    typedef std::list<INode *> NodeList ;
    typedef std::list<IProtocol *> ProtocolList ;
public:
	virtual ~Kernel () ;

	void RegisterNode (INode * p) { this->_nodes.push_back (p ); }
	void RegisterProtocol (IProtocol * p) { this->_protocols.push_back (p ); }

	void	LoadPlugins (std::string & dir) ;
	NodeList & 			Nodes () ;
	ProtocolList & 	Protocols () ;
	PluginMap & 	Plugins () ;
	INode *		Node (std::string name) ;
	IProtocol *		Protocol (std::string name) ;
	
	typedef int	(*tPluginFonction) (Kernel & ) ;


private:
	NodeList		_nodes ;
	ProtocolList	_protocols ;

	template <class T>
    void Free (std::list<T*> & l) {
      typename std::list<T*>::reverse_iterator it ;
      for (it = l.rbegin(); it != l.rend(); ++it
      ) {
        delete *it;
      }
    }

    /// <summary>Map of plugins by their associated file names</summary>
    private: PluginMap _loadedPlugins;
	
} ;

} ; // namespace ydle
#endif //  _Plugin_H_
