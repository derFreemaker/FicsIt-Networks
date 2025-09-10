#include "FINLuaUtil.h"

namespace FINLua {
	FString luaFIN_toFString(lua_State* L, int index) {
		size_t len;
		const char* str = lua_tolstring(L, index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}
}
