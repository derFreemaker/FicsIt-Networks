#include "FINLua/LuaUtil.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLua/Reflection/LuaClass.h"
#include "FINLua/LuaFuture.h"
#include "FINLua/LuaPersistence.h"
#include "FINLua/Reflection/LuaObject.h"
#include "FINLua/Reflection/LuaStruct.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

namespace FINLua {
	void luaFIN_pushNetworkValue(lua_State* L, const FFINAnyNetworkValue& Val, const FFINNetworkTrace& Trace) {
		switch (Val.GetType()) {
		case FIN_NIL:
			lua_pushnil(L);
			break;
		case FIN_BOOL:
			lua_pushboolean(L, Val.GetBool());
			break;
		case FIN_INT:
			lua_pushinteger(L, Val.GetInt());
			break;
		case FIN_FLOAT:
			lua_pushnumber(L, Val.GetFloat());
			break;
		case FIN_STR: {
			luaFIN_pushFString(L, Val.GetString());
			break;
		} case FIN_OBJ:
			luaFIN_pushObject(L, Trace / Val.GetObj().Get());
			break;
		case FIN_CLASS:
			luaFIN_pushClass(L, Val.GetClass());
			break;
		case FIN_TRACE:
			luaFIN_pushObject(L, Val.GetTrace());
			break;
		case FIN_STRUCT: {
			const FINStruct& Struct = Val.GetStruct();
			if (Struct.GetStruct()->IsChildOf(FFINFuture::StaticStruct())) {
				luaFuture(L, Struct);
			} else {
				luaFIN_pushStruct(L, Val.GetStruct());
			}
			break;
		} case FIN_ARRAY: {
			lua_newtable(L);
			int i = 0;
			for (const FFINAnyNetworkValue& Entry : Val.GetArray()) {
				luaFIN_pushNetworkValue(L, Entry, Trace);
				lua_seti(L, -2, ++i);
			}
			break;
		} case FIN_ANY:
			luaFIN_pushNetworkValue(L, Val.GetAny(), Trace);
			lua_pushnil(L);
			break;
		default:
			lua_pushnil(L);
		}
	}

	TOptional<EFINNetworkValueType> luaFIN_getNetworkValueType(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FIN_NIL;
		case LUA_TBOOLEAN:
			return FIN_BOOL;
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FIN_INT;
			} else {
				return FIN_FLOAT;
			}
		case LUA_TSTRING:
			return FIN_STR;
		case LUA_TUSERDATA:
			if (luaFIN_toLuaStruct(L, Index, nullptr)) return FIN_STRUCT;
			if (luaFIN_toLuaObject(L, Index, nullptr)) return FIN_TRACE;
			if (luaFIN_toLuaClass(L, Index)) return FIN_CLASS;
		default: return TOptional<EFINNetworkValueType>();
		}
	}

	TOptional<FINAny> luaFIN_toNetworkValueByProp(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		int LuaType = lua_type(L, Index);
		
		switch (Property->GetType()) {
		case FIN_NIL:
			return FINAny();
		case FIN_BOOL:
			if (!bImplicitConversion && LuaType != LUA_TBOOLEAN) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinBool(L, Index));
		case FIN_INT:
			if (!bImplicitConversion && (LuaType != LUA_TNUMBER || !lua_isinteger(L, Index))) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinInt(L, Index));
		case FIN_FLOAT:
			if (!bImplicitConversion && LuaType != LUA_TNUMBER) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinFloat(L, Index));
		case FIN_STR: {
			if (!bImplicitConversion && LuaType != LUA_TSTRING) return TOptional<FINAny>();
			return FINAny(luaFIN_toFinString(L, Index));
		} case FIN_OBJ: {
			TOptional<FINTrace> Object;
			UFINObjectProperty* ObjectProp = Cast<UFINObjectProperty>(Property);
			if (ObjectProp && ObjectProp->GetSubclass()) {
				Object = luaFIN_toObject(L, Index, FFINReflection::Get()->FindClass(ObjectProp->GetSubclass()));
				if (bImplicitConversion && !Object.IsSet() && UFINClass::StaticClass()->IsChildOf(ObjectProp->GetSubclass())) {
					UClass* Class = luaFIN_toUClass(L, Index, nullptr);
					if (Class) Object = FINTrace(Class);
				}
			} else {
				Object = luaFIN_toObject(L, Index, nullptr);
				if (bImplicitConversion && !Object.IsSet()) {
					UClass* Class = luaFIN_toUClass(L, Index, nullptr);
					if (Class) Object = FINTrace(Class);
				}
			}
			if (Object.IsSet()) return FINAny(static_cast<FINObj>(Object.GetValue().Get()));
			return TOptional<FINAny>();
		} case FIN_CLASS: {
			UClass* Class;
			UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property);
			if (ClassProp && ClassProp->GetSubclass()) {
				Class = luaFIN_toSubUClass(L, Index, ClassProp->GetSubclass());
			} else {
				Class = luaFIN_toUClass(L, Index, nullptr);
			}
			if (Class) return FINAny(static_cast<FINClass>(Class));
			return TOptional<FINAny>();
		} case FIN_TRACE: {
			TOptional<FINTrace> Trace;
			UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property);
			if (TraceProp && TraceProp->GetSubclass()) {
				Trace = luaFIN_checkObject(L, Index, FFINReflection::Get()->FindClass(TraceProp->GetSubclass()));
			} else {
				Trace = luaFIN_toObject(L, Index, nullptr);
			}
			if (Trace.IsSet()) return FINAny(Trace.GetValue());
			return TOptional<FINAny>();
		} case FIN_STRUCT: {
			TSharedPtr<FINStruct> Struct;
			UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property);
			if (StructProp && StructProp->GetInner()) {
				UFINStruct* Type = FFINReflection::Get()->FindStruct(StructProp->GetInner());
				Struct = luaFIN_checkStruct(L, Index, Type, bImplicitConstruction);
			} else {
				Struct = luaFIN_toStruct(L, Index, nullptr, false);
			}
			if (Struct.IsValid()) return FINAny(*Struct);
			return TOptional<FINAny>();
		} case FIN_ARRAY: {
			if (LuaType != LUA_TTABLE) return TOptional<FINAny>();
			UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property);
			FINArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FINAny> Value;
				if (ArrayProp && ArrayProp->GetInnerType()) {
					Value = luaFIN_toNetworkValueByProp(L, -1, ArrayProp->GetInnerType(), bImplicitConversion, bImplicitConstruction);
				} else {
					Value = luaFIN_toNetworkValue(L, -1);
				}
				if (Value.IsSet()) Array.Add(Value.GetValue());
				lua_pop(L, 1);
			}
			return FINAny(Array);
		} case FIN_ANY: return luaFIN_toNetworkValue(L, Index);
		default: ;
		}
		return FINAny();
	}
	
	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index, UFINProperty* Property, bool bImplicitConversion, bool bImplicitConstruction) {
		if (Property) return luaFIN_toNetworkValueByProp(L, Index, Property, bImplicitConversion, bImplicitConstruction);
		else return luaFIN_toNetworkValue(L, Index);
	}

	TOptional<FINAny> luaFIN_toNetworkValue(lua_State* L, int Index) {
		switch (lua_type(L, Index)) {
		case LUA_TNIL:
			return FINAny();
		case LUA_TBOOLEAN:
			return FINAny(luaFIN_toFinBool(L, Index));
		case LUA_TNUMBER:
			if (lua_isinteger(L, Index)) {
				return FINAny(luaFIN_toFinInt(L, Index));
			} else {
				return FINAny(luaFIN_toFinFloat(L, Index));
			}
		case LUA_TSTRING:
			return FINAny(luaFIN_toFinString(L, Index));
		case LUA_TTABLE: {
			FINArray Array;
			lua_pushnil(L);
			while (lua_next(L, Index) != 0) {
				if (!lua_isinteger(L, -2)) break;
				TOptional<FINAny> Value = luaFIN_toNetworkValue(L, -1);
				lua_pop(L, 1);
				if (!Value.IsSet()) return TOptional<FINAny>();
				Array.Add(*Value);
			}
			return FINAny(Array);
		} default:
			TSharedPtr<FINStruct> Struct = luaFIN_toStruct(L, Index, nullptr, false);
			if (Struct.IsValid()) return FINAny(static_cast<FINStruct>(*Struct));
			TOptional<FFINNetworkTrace> Object = luaFIN_toObject(L, Index, nullptr);
			if (Object.IsSet()) return FINAny(static_cast<FINObj>(Object.GetValue().Get()));
			UClass* Class = luaFIN_toUClass(L, Index, nullptr);
			if (Class) return FINAny(static_cast<FINClass>(Class));
		}
		return TOptional<FINAny>();
	}

	FString luaFIN_getPropertyTypeName(lua_State* L, UFINProperty* Property) {
		switch (Property->GetType()) {
		case FIN_NIL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TNIL));
		case FIN_BOOL: return UTF8_TO_TCHAR(lua_typename(L, LUA_TBOOLEAN));
		case FIN_INT: return TEXT("integer");
		case FIN_FLOAT: return TEXT("float");
		case FIN_STR: return UTF8_TO_TCHAR(lua_typename(L, LUA_TSTRING));
		case FIN_OBJ: {
			UFINObjectProperty* ObjProp = Cast<UFINObjectProperty>(Property);
			UFINClass* Class = nullptr;
			if (ObjProp && ObjProp->GetSubclass()) {
				Class = FFINReflection::Get()->FindClass(ObjProp->GetSubclass());
			}
			return FFINReflection::ObjectReferenceText(Class);
		} case FIN_CLASS: {
			UFINClassProperty* ClassProp = Cast<UFINClassProperty>(Property);
			UFINClass* Class = nullptr;
			if (ClassProp && ClassProp->GetSubclass()) {
				Class = FFINReflection::Get()->FindClass(ClassProp->GetSubclass());
			}
			return FFINReflection::ClassReferenceText(Class);
		} case FIN_TRACE: {
			UFINTraceProperty* TraceProp = Cast<UFINTraceProperty>(Property);
			UFINClass* Class = nullptr;
			if (TraceProp && TraceProp->GetSubclass()) {
				Class = FFINReflection::Get()->FindClass(TraceProp->GetSubclass());
			}
			return FFINReflection::TraceReferenceText(Class);
		} case FIN_STRUCT: {
			UFINStructProperty* StructProp = Cast<UFINStructProperty>(Property);
			UFINStruct* Type = nullptr;
			if (StructProp && StructProp->GetInner()) {
				Type = FFINReflection::Get()->FindStruct(StructProp->GetInner());
			}
			return FFINReflection::StructReferenceText(Type);
		} case FIN_ARRAY: {
			FString TypeName = TEXT("Array");
			UFINArrayProperty* ArrayProp = Cast<UFINArrayProperty>(Property);
			if (ArrayProp && ArrayProp->GetInnerType()) {
				TypeName.Append(TEXT("<")).Append(luaFIN_getPropertyTypeName(L, ArrayProp->GetInnerType())).Append(TEXT(">"));
			}
			return TypeName;
		} case FIN_ANY: return TEXT("Any");
		default: ;
		}
		return TEXT("Unkown");
	}
	
	int luaFIN_propertyError(lua_State* L, int Index, UFINProperty* Property) {
		return luaFIN_typeError(L, Index, luaFIN_getPropertyTypeName(L, Property));
	}

	int luaFIN_typeError(lua_State* L, int Index, const FString& ExpectedTypeName) {
		FString ActualTypeName = luaFIN_typeName(L, Index);
		return luaFIN_argError(L, Index, FString::Printf(TEXT("%s expected, got %s"), *ExpectedTypeName, *ActualTypeName));
	}

	int luaFIN_argError(lua_State* L, int Index, const FString& ExtraMessage) {
		return luaL_argerror(L, Index, TCHAR_TO_UTF8(*ExtraMessage));
	}

	FString luaFIN_typeName(lua_State* L, int Index) {
		const char *typearg;
		if (luaL_getmetafield(L, Index, "__name") == LUA_TSTRING) {
			typearg = lua_tostring(L, -1);
		} else if (lua_type(L, Index) == LUA_TLIGHTUSERDATA) {
			typearg = "light userdata";
		} else {
			typearg = luaL_typename(L, Index);
		}
		FString TypeName = UTF8_TO_TCHAR(typearg);
		if (TypeName == FIN_LUA_OBJECT_METATABLE_NAME) {
			FLuaObject* LuaObject = luaFIN_toLuaObject(L, Index, nullptr);
			UFINClass* Type = nullptr;
			if (LuaObject) Type = LuaObject->Type;
			return FFINReflection::ObjectReferenceText(Type);
		}
		if (TypeName == FIN_LUA_CLASS_METATABLE_NAME) {
			FLuaClass* LuaClass = luaFIN_toLuaClass(L, Index);
			UFINClass* Type = nullptr;
			if (LuaClass) Type = LuaClass->FINClass;
			return FFINReflection::ClassReferenceText(Type);
		}
		if (TypeName == FIN_LUA_STRUCT_METATABLE_NAME) {
			FLuaStruct* LuaStruct = luaFIN_toLuaStruct(L, Index, nullptr);
			UFINStruct* Type = nullptr;
			if (LuaStruct) Type = LuaStruct->Type;
			return FFINReflection::StructReferenceText(Type);
		}
		return TypeName;
	}

	FString luaFIN_getUserDataMetaName(lua_State* L, int Index) {
		if (lua_type(L, Index) != LUA_TUSERDATA) return FString();
		int fieldType = luaL_getmetafield(L, Index, "__name");
		if (fieldType != LUA_TSTRING) {
			if (fieldType != LUA_TNONE) lua_pop(L, 1);
			return FString();
		}
		FString metaName = luaFIN_toFString(L, -1);
		lua_pop(L, 1);
		return metaName;
	}

	void luaFIN_pushFString(lua_State* L, const FString& Str) {
		FTCHARToUTF8 conv(*Str, Str.Len());
		lua_pushlstring(L, conv.Get(), conv.Length());
	}

	FString luaFIN_checkFString(lua_State* L, int Index) {
		size_t len;
		const char* str = luaL_checklstring(L, Index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	FString luaFIN_toFString(lua_State* L, int index) {
		size_t len;
		const char* str = lua_tolstring(L, index, &len);
		FUTF8ToTCHAR conv(str, len);
		return FString(conv.Length(), conv.Get());
	}

	void luaFIN_warning(lua_State* L, const char* msg, int tocont) {
		lua_Debug ar;
		if (lua_getstack(L, 1, &ar)) {
			lua_getinfo(L, "Sl", &ar);
			if (ar.currentline > 0) {
				lua_pushfstring(L, "%s:%d: %s", ar.short_src, ar.currentline, msg);
				const char* warn = lua_tostring(L, -1);
				lua_warning(L, warn, tocont);
				lua_pop(L, 1);
				return;
			}
		}
		lua_warning(L, msg, tocont);
	}

	FString luaFIN_where(lua_State* L) {
		lua_Debug ar;
		if (lua_getstack(L, 1, &ar)) {
			lua_getinfo(L, "Sl", &ar);
			if (ar.currentline > 0) {
				return FString::Printf(TEXT("%s:%d"), UTF8_TO_TCHAR(ar.short_src), ar.currentline);
			}
		}
		return FString();
	}

	FString luaFIN_stack(lua_State* L) {
		return FString();
	}

	void luaFINDebug_dumpStack(lua_State* L) {
		int args = lua_gettop(L);
		int negative = 0;
		for (; args > 0; --args) {
			UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Lua Stack: [%i/%i] %s"), args, --negative, *luaFIN_typeName(L, args));
		}
	}

	void setupUtilLib(lua_State* L) {
		PersistenceNamespace("UtilLib");
		
		
	}
}

FFINLuaLogScope::FFINLuaLogScope(lua_State* L) : FFINLogScope(nullptr, FWhereFunction::CreateLambda([L]() {
	return FINLua::luaFIN_where(L);
}), FStackFunction::CreateLambda([L]() {
	return FINLua::luaFIN_stack(L);
})) {}

FCbWriter& operator<<(FCbWriter& Writer, lua_State* const& L) {
	Writer.AddString(FINLua::luaFIN_where(L));
	return Writer;
}
