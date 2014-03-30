/*
 * LuaStack.h
 *
 *  Created on: Jan 31, 2014
 *      Author: denia
 */
#include <lua5.1/lua.hpp>
#include <string>
#include <map>

#ifndef _LuaStack_H_
#define _LuaStack_H_

namespace ydle {

	typedef std::map <std::string, double> tMapVal;

class LuaStack {
public:
	LuaStack();
	LuaStack(std::string nom);
	virtual ~LuaStack();
	void setScript(std::string script);
	void	init();
	void	action();
	lua_State*	State()  {return L ;}
	const char * NameStr() {return name.c_str() ;}
private:
	static int l_getVal (lua_State*) ;
	static int l_setVal (lua_State*) ;
//FETS		static int l_getTime (lua_State*) ;
	lua_State *L;
	std::string	name;
};

} /* namespace ydle */

#endif /* _LuaStack_H_ */
