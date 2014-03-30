/*
 * LuaStack.h
 *
 *  Created on: Jan 31, 2014
 *      Author: denia
 */

#ifndef _Scripts_H_
#define _Scripts_H_

#include <string>
#include "ListPtr.h"
#include "IFeature.h"
#include "LuaStack.h"
#include "Thread.h"
#include "DynLoad.h"

namespace ydle {

class PluginLua
{
public:
	PluginLua (std::string &) ;
	virtual ~PluginLua ()  {};
	bool		Register (lua_State *) ;
private:
	DynLoad mDyn ;
} ;

class Scripts : public ListPtr<LuaStack>, public IFeature, public Thread {
public:
	Scripts (std::string nom) ;
	virtual ~Scripts () ;
	void			LoadScript () ;
	void			Plugins () ;
	void			AddScript (const char * name) ;
	virtual void Run() ;
	virtual void Init() ;
	virtual void Start() ;
protected:
	virtual void ThreadBegin () ;
	virtual void ThreadAction () ;
	virtual void ThreadEnd () ;

    typedef std::map<std::string, PluginLua*> PluginMap;
    private: PluginMap _loadedPlugins;
	
};

} /* namespace ydle */

#endif /* _Scripts_H_ */
