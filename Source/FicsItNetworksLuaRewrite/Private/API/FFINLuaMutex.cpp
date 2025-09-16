#include "FFINLuaMutex.h"

#define LUA_MUTEX_TNAME "FIN.lua.mutex"

static int LuaMutexTryLock(lua_State *L) {
	bool *Mutex = static_cast<bool*>(luaL_checkudata(L, lua_upvalueindex(1), LUA_MUTEX_TNAME));

	if (*Mutex) {
		lua_pushboolean(L, false);
		return 1;
	}
	*Mutex = true;

	lua_pushboolean(L, true);
	return 1;
}

static int LuaMutexUnlock(lua_State *L) {
	bool *Mutex = static_cast<bool*>(luaL_checkudata(L, lua_upvalueindex(1), LUA_MUTEX_TNAME));
	*Mutex = false;
	return 0;
}


int LuaMutex(lua_State *L) {
	bool *Mutex = static_cast<bool*>(lua_newuserdatauv(L, sizeof(bool), 0));
	*Mutex = false;

	if (luaL_newmetatable(L, LUA_MUTEX_TNAME)) {
		lua_pushvalue(L, -2); // copy mutex userdata
		lua_pushcclosure(L, &LuaMutexTryLock, 1);
		lua_setfield(L, -2, "trylock");

		lua_pushvalue(L, -2); // copy mutex userdata
		lua_pushcclosure(L, &LuaMutexUnlock, 1);
		lua_setfield(L, -2, "unlock");
	}

	lua_setmetatable(L, -2);
	return 1;
}
