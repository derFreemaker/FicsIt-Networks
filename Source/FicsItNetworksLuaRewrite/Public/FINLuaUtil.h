#pragma once

#include "FILLogScope.h"
#include "FINLua.h"
#include "FIRTypes.h"

class UFIRProperty;
class UFIRStruct;
class UFIRFunction;

namespace FINLua {
	FICSITNETWORKSLUAREWRITE_API FString luaFIN_toFString(lua_State* L, int index);	
}
