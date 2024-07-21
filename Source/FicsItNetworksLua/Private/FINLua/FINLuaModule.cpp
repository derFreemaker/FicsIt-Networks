﻿#include "FINLua/FINLuaModule.h"

#include "FicsItNetworksLuaModule.h"
#include "FINLua/LuaExtraSpace.h"
#include "FINLua/LuaPersistence.h"
#include "Logging/StructuredLog.h"

FText CreateLocalizedYAY(FStringView NameSpace, FStringView Key, FStringView Text) {
	return FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(Text.GetData(), NameSpace.GetData(), Key.GetData());
}

TTuple<FString, TArray<TTuple<FString, TArray<FString>>>> PreprocessDocumentationComment(FStringView Comment) {
	FString block;
	TArray<TTuple<FString, TArray<FString>>> options;

	while (Comment.Len() > 0) {
		FStringView line;
		int32 linebreak;
		if (Comment.FindChar('\n', linebreak)) {
			line = Comment.SubStr(0, linebreak);
			Comment = Comment.RightChop(linebreak+1);
		} else {
			line = Comment;
			Comment = FStringView();
		}

		line.TrimStartAndEndInline();
		while (line.StartsWith('*') || line.StartsWith('/')) line.RightChopInline(1);
		line.TrimStartAndEndInline();

		if (line.IsEmpty()) {
			continue;
		}

		static FRegexPattern OptionPattern(TEXT("@(\\w+)((\\t+([^\\t]+))+)"));
		FRegexMatcher matchOption(OptionPattern, FString(line));
		if (matchOption.FindNext()) {
			FString option = matchOption.GetCaptureGroup(1);
			TArray<FString>& parameters = options.Emplace_GetRef(option, TArray<FString>()).Value;
			FString parameterString = matchOption.GetCaptureGroup(2);

			static FRegexPattern OptionParameterPattern(TEXT("\\t+([^\\t]+)"));
			FRegexMatcher matchParameter(OptionParameterPattern, parameterString);
			while (matchParameter.FindNext()) {
				FString parameter = matchParameter.GetCaptureGroup(1);
				parameters.Add(parameter.TrimStartAndEnd());
			}
		} else {
			if (block.Len() > 0) block += '\n';
			block += line;
		}
	}

	return {block, options};
}

void FFINLuaModuleBareValue::PushLuaValue(lua_State* L, const FString& PersistName) {
	Function(L, PersistName);
}

void FFINLuaFunction::PushLuaValue(lua_State* L, const FString& PersistName) {
	lua_pushnil(L);
	lua_pushcclosure(L, Function, 1);

	FINLua::luaFIN_persistValue(L, -1, PersistName);
}

void FFINLuaTable::PushLuaValue(lua_State* L, const FString& PersistName) {
	lua_createtable(L, 0, Fields.Num());	// table

	for (FFINLuaTableField& field : Fields) {
		if (!field.Value.IsValid()) continue;

		FINLua::luaFIN_pushFString(L, field.Key);	// table, string
		field.Value->PushLuaValue(L, PersistName + TEXT("-") + field.Key);	// table, string, value

		if (field.Value->TypeID() == FINTypeId<FFINLuaFunction>::ID() && PersistName != TEXT("ModuleSystem-Metatable-ModuleTableFunction")) {
			lua_pushlightuserdata(L, &field);
			lua_setupvalue(L, -2, 1);
			luaL_setmetatable(L, "ModuleTableFunction");
		}

		lua_settable(L, -3);	// table
	}
}

void FFINLuaTable::AddFunctionFieldByDocumentationComment(lua_CFunction Function, const FString& Comment, const TCHAR* InternalName) {
	FFINLuaTableField& field = Fields.Emplace_GetRef();
	auto func = MakeShared<FFINLuaFunction>(Function);
	field.Value = func;
	field.Key = InternalName;

	auto [block, options] = PreprocessDocumentationComment(Comment);

	for (auto [option, parameters] : options) {
		if (option.Equals(TEXT("LuaFunction"), ESearchCase::IgnoreCase)) {
			if (parameters.Num() > 2) {
				func->ReturnValueSignature = parameters[0];
				field.Key = parameters[1];
				func->ParameterSignature = parameters[2];
			} else {
				if (parameters.Num() > 1) {
					func->ReturnValueSignature = parameters[0];
					field.Key = parameters[1];
				} else {
					field.Key = parameters[0];
				}
				int32 parenthesis;
				if (field.Key.FindChar('(', parenthesis)) {
					func->ParameterSignature = field.Key.RightChop(parenthesis + 1);
					func->ParameterSignature.RemoveFromEnd(TEXT(")"));
					field.Key = field.Key.Left(parenthesis);
				}
			}
			if (field.DisplayName.IsEmpty()) {
				field.DisplayName = FText::FromString(field.Key);
			}
		} else if (option.Equals(TEXT("DisplayName"), ESearchCase::IgnoreCase)) {
			field.DisplayName = FText::FromString(parameters[0]);
		} else if (option.Equals(TEXT("parameter"), ESearchCase::IgnoreCase)) {
			FFINLuaFunctionParameter& param = func->Parameters.Emplace_GetRef();
			param.InternalName = parameters[0];
			if (parameters.Num() > 1) {
				param.Type = parameters[1];
			}
			if (parameters.Num() > 2) {
				param.DisplayName = FText::FromStringView(parameters[2]);
			} else {
				param.DisplayName = FText::FromString(param.InternalName);
			}
			if (parameters.Num() > 3) {
				param.Description = FText::FromStringView(parameters[3]);
			}
		} else if (option.Equals(TEXT("return"), ESearchCase::IgnoreCase)) {
			FFINLuaFunctionParameter& retVal = func->ReturnValues.Emplace_GetRef();
			retVal.InternalName = parameters[0];
			if (parameters.Num() > 1) {
				retVal.Type = parameters[1];
			}
			if (parameters.Num() > 2) {
				retVal.DisplayName = FText::FromStringView(parameters[2]);
			} else {
				retVal.DisplayName = FText::FromString(retVal.InternalName);
			}
			if (parameters.Num() > 3) {
				retVal.Description = FText::FromStringView(parameters[3]);
			}
		}
	}

	field.Description = FText::FromString(block);
}

void FFINLuaTable::AddBareFieldByDocumentationComment(TFunction<void(lua_State* L, const FString&)> Function, const FString& Comment, const TCHAR* InternalName) {
	FFINLuaTableField& field = Fields.Emplace_GetRef();
	auto bare = MakeShared<FFINLuaModuleBareValue>(Function);
	field.Value = bare;
	field.Key = InternalName;

	auto [block, options] = PreprocessDocumentationComment(Comment);

	for (auto [option, parameters] : options) {
		if (option.Equals(TEXT("LuaBareField"), ESearchCase::IgnoreCase)) {
			field.Key = parameters[0];
			if (field.DisplayName.IsEmpty()) {
				if (parameters.Num() > 1) {
					field.DisplayName = FText::FromString(parameters[1]);
				} else {
					field.DisplayName = FText::FromString(InternalName);
				}
			}
		} else if (option.Equals(TEXT("DisplayName"), ESearchCase::IgnoreCase)) {
			field.DisplayName = FText::FromString(parameters[0]);
		}
	}

	field.Description = FText::FromString(TEXT("Description"));
}

void FFINLuaModule::SetupModule(lua_State* L) {
	PreSetup.ExecuteIfBound(*this, L);

	PersistenceNamespace(InternalName);

	for (const FFINLuaMetatable& metatable : Metatables) {
		FINLua::luaFIN_pushFString(L, metatable.InternalName);	// string
		lua_pushvalue(L, -1);	// string, string
		if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TNIL) {	// string, nil|any
			lua_pop(L, 2);	//
			UE_LOGFMT(LogFicsItNetworksLuaPersistence, Warning, "Failed to create Metatable '{MetatableName}' of Module '{ModuleName}'! Probably already exists!", metatable.InternalName, InternalName);
			continue;
		}
		lua_pop(L, 1);	// string

		metatable.Table->PushLuaValue(L, InternalName + TEXT("-Metatable-") + metatable.InternalName); // string, Metatable

		lua_pushvalue(L, -2);	// string, Metatable, string
		lua_setfield(L, -2, "__name");	// string, Metatable
		lua_pushvalue(L, -2); // string, Metatable, string
		lua_pushvalue(L, -2); // string, Metatable, string, Metatable
		lua_settable(L, LUA_REGISTRYINDEX); // string, Metatable

		//FINLua::luaFIN_pushFString(L, Metatable.InternalName);
		//lua_setfield(L, -2, "__metatable);

		PersistValue(metatable.InternalName + TEXT("-Metatable"));	// string
		lua_pop(L, 1); //
	}

	lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);	// Globals
	for (const FFINLuaGlobal& global : Globals) {
		FINLua::luaFIN_pushFString(L, global.InternalName);	// Globals, string

		lua_pushvalue(L, -1);	// Globals, string, string
		if (lua_gettable(L, -2) > 0) {	// Globals, string, nil|any
			lua_pop(L, 2); //
			UE_LOGFMT(LogFicsItNetworksLuaPersistence, Warning, "Failed to add Global Library '{LibraryName}' of Module '{ModuleName}'! Probably already exists!", global.InternalName, InternalName);
			continue;
		}
		lua_pop(L, 1);	// Globals, string

		global.Value->PushLuaValue(L, InternalName + TEXT("-Global-") + global.InternalName);	// Globals, string, Value

		lua_settable(L, 1);	// Globals
	}
	lua_pop(L, 1);	//

	PostSetup.ExecuteIfBound(*this, L);

	FINLua::luaFIN_getExtraSpace(L).LoadedModules.Add(AsShared());
}

void FFINLuaModule::ParseDocumentationComment(const FString& Comment, const TCHAR* _InternalName) {
	InternalName = _InternalName;

	auto [block, options] = PreprocessDocumentationComment(Comment);

	for (auto [option, parameters] : options) {
		if (option.Equals(TEXT("LuaModule"), ESearchCase::IgnoreCase)) {
			InternalName = parameters[0];
			if (DisplayName.IsEmpty()) {
				if (parameters.Num() > 1) {
					DisplayName = FText::FromString(parameters[1]);
				} else {
					DisplayName = FText::FromString(InternalName);
				}
			}
		} else if (option.Equals(TEXT("DisplayName"), ESearchCase::IgnoreCase)) {
			DisplayName = FText::FromString(parameters[0]);
		}
	}

	Description = FText::FromString(TEXT("Description"));
}

void FFINLuaModule::AddLibraryByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* _InternalName) {
	FFINLuaGlobal& global = Globals.Emplace_GetRef();
	global.Value = Table;

	global.InternalName = _InternalName;

	auto [block, options] = PreprocessDocumentationComment(Comment);

	for (auto [option, parameters] : options) {
		if (option.Equals(TEXT("LuaLibrary"), ESearchCase::IgnoreCase)) {
			global.InternalName = parameters[0];
			if (global.DisplayName.IsEmpty()) {
				if (parameters.Num() > 1) {
					global.DisplayName = FText::FromString(parameters[1]);
				} else {
					global.DisplayName = FText::FromString(global.InternalName);
				}
			}
		} else if (option.Equals(TEXT("DisplayName"), ESearchCase::IgnoreCase)) {
			global.DisplayName = FText::FromString(parameters[0]);
		}
	}

	global.Description = FText::FromString(block);
}

void FFINLuaModule::AddMetatableByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* _InternalName) {
	FFINLuaMetatable& metatable = Metatables.Emplace_GetRef();
	metatable.Table = Table;

	metatable.InternalName = _InternalName;

	auto [block, options] = PreprocessDocumentationComment(Comment);

	for (auto [option, parameters] : options) {
		if (option.Equals(TEXT("LuaMetatable"), ESearchCase::IgnoreCase)) {
			metatable.InternalName = parameters[0];
			if (metatable.DisplayName.IsEmpty()) {
				if (parameters.Num() > 1) {
					metatable.DisplayName = FText::FromString(parameters[1]);
				} else {
					metatable.DisplayName = FText::FromString(metatable.InternalName);
				}
			}
		} else if (option.Equals(TEXT("DisplayName"), ESearchCase::IgnoreCase)) {
			metatable.DisplayName = FText::FromString(parameters[0]);
		}
	}

	metatable.Description = FText::FromString(block);
}

FFINLuaModuleRegistry& FFINLuaModuleRegistry::GetInstance() {
	static FFINLuaModuleRegistry registry;
	return registry;
}

namespace FINLua {
	LuaModule(R"(/**
	 * @LuaModule		ModuleSystem
	 * @DisplayName		Module-System Module
	 */)", ModuleSystem) {
		LuaModuleMetatable(R"(/**
		 * @LuaMetatable	ModuleTableFunction
		 * @DisplayName		Module Table-Function)", ModuleTableFunction) {
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	name
			 * @DisplayName		Name
			 */)", name) { lua_pushnil(L); }
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	displayName
			 * @DisplayName		Display Name
			 */)", displayName) { lua_pushnil(L); }
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	description
			 * @DisplayName		Description
			 */)", description) { lua_pushnil(L); }
			LuaModuleTableBareField(R"(/**
			 * @LuaBareField	quickRef
			 * @DisplayName		Quick Reference
			 */)", quickRef) { lua_pushnil(L); }

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__index
			 * @DisplayName		Index
			 */)", __index) {
				lua_getupvalue(L, 1, 1);
				if (!lua_isuserdata(L, -1)) {
					return 0;
				}
				FFINLuaTableField* field = static_cast<FFINLuaTableField*>(lua_touserdata(L, -1));

				FString key = luaFIN_checkFString(L, 2);
				if (key == TEXT("name")) {
					luaFIN_pushFString(L, field->Key);
				} else if (key == TEXT("quickRef")) {
					FString head = field->Key;
					if (field->Value->TypeID() == FINTypeId<FFINLuaFunction>::ID()) {
						auto func = StaticCastSharedPtr<FFINLuaFunction>(field->Value);
						head = func->GetSignature(field->Key);
					}

					FString str = FString::Printf(TEXT("# %ls\n%ls"), *head, *field->Description.ToString());

					luaFIN_pushFString(L, str);
				} else {
					return 0;
				}

				return 1;
			}

			LuaModuleTableFunction(R"(/**
			 * @LuaFunction		__tostring
			 * @DisplayName		To String
			 */)", __tostring) {
				lua_getupvalue(L, 1, 1);
				if (!lua_isuserdata(L, -1)) {
					return 0;
				}
				FFINLuaTableField* field = static_cast<FFINLuaTableField*>(lua_touserdata(L, -1));
				FString head = field->Key;
				if (field->Value->TypeID() == FINTypeId<FFINLuaFunction>::ID()) {
					auto func = StaticCastSharedPtr<FFINLuaFunction>(field->Value);
					head = func->GetSignature(field->Key);
				}
				luaFIN_pushFString(L, FString::Printf(TEXT("function: %ls"), *head));
				return 1;
			}
		}
	}
}