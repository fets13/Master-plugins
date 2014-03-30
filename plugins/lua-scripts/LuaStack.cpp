/*
 * LuaStack.cpp
 *
 *  Created on: Jan 31, 2014
 *      Author: denia
 */
#include <iostream>
#include <sys/time.h>

#include "LuaStack.h"
#include "DataAccess.h"

#define DDD printf ("l:%d gettop=%d\n", __LINE__, lua_gettop(L));
using namespace ydle ;

int LuaStack::l_getVal(lua_State *L)
{
  const char * nom = luaL_checkstring(L, -1);
  double val ;
  if (DATA.Get(nom,val)) {
	  lua_pushnumber(L, val) ;
	  return 1;
  }
  else return 0 ;

}

int LuaStack::l_setVal(lua_State *L)
{
  const char * nom = luaL_checkstring(L, 1);
  double val = luaL_checknumber(L, 2);
//FETS	  printf ("Set(%s)=%g\n", nom, val);
  DATA[nom] = val ;
  return 1;
}

#if 0
int LuaStack::l_getTime(lua_State *L)
{
	struct timeval localTime;
	gettimeofday(&localTime, NULL); 
	lua_pushnumber(L, localTime.tv_sec) ;
	return 1;
}
#endif


LuaStack::LuaStack()
{
	L = luaL_newstate();
}

LuaStack::LuaStack(std::string script)
{
	L = luaL_newstate();
	setScript (script);
}

void LuaStack::init()
{
    lua_getglobal(L, "init");  /* function to be called */
    // No args, no return
    lua_pcall(L, 0, 0, 0);
}

void LuaStack::action()
{
    lua_getglobal(L, "action");  /* function to be called */
    // No args, no return
    lua_pcall(L, 0, 0, 0);
}

void LuaStack::setScript(std::string script)
{
	luaL_openlibs(L);

#define DECL_LUA_FUNC(nom)	lua_pushcfunction(L, l_##nom);	\
	lua_setglobal(L, #nom);

	DECL_LUA_FUNC(getVal);
	DECL_LUA_FUNC(setVal);
//FETS		DECL_LUA_FUNC(getTime);

	name = script ;

	if (luaL_loadfile(L, name.c_str())) {
		std::cerr << "Something went wrong loading the chunk (syntax error?)" << std::endl;
		std::cerr << lua_tostring(L, -1) << std::endl;
		lua_pop(L,1);
	}

	if (lua_pcall(L,0, LUA_MULTRET, 0)) {
		std::cerr << "run: Something went wrong during execution" << std::endl;
		std::cerr << lua_tostring(L, -1) << std::endl;
		lua_pop(L,1);
	}
}

LuaStack::~LuaStack()
{
	lua_close(L);
}
