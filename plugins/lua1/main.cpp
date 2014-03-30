
#include <lua5.1/lua.hpp>

//FETS	using namespace ydle ;
#include <sys/time.h>


extern "C" {

int l_testPlugin(lua_State *L)
{
	printf ("dans testPlugin\n") ;
	return 0;
}
int l_getTime(lua_State *L)
{
	struct timeval localTime;
	gettimeofday(&localTime, NULL); 
	lua_pushnumber(L, localTime.tv_sec) ;
	return 1;
}


bool	LoadLua (lua_State * l)
{
#define DECL_LUA_FUNC(nom)	lua_pushcfunction(l, l_##nom);	\
	lua_setglobal(l, #nom);

	DECL_LUA_FUNC(testPlugin);
	DECL_LUA_FUNC(getTime);
	return true ;

}
}
