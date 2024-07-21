#pragma once

#include "FINGlobalRegisterHelper.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStaticReflectionSource.h"

#include "ThirdParty/fameta_counter.hpp"

#define TypeClassName(Type) FIN_StaticRef_ ## Type
#define NSName "FicsItNetworks-StaticReflection"
#define FINRefLocText(KeyName, Value) FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(Value, TEXT(NSName), KeyName)
#define FINRefTypeLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginClass(Type, InternalName, DisplayName, Description) \
	namespace TypeClassName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(InternalName) ; \
		UClass* GetUType() { return T::StaticClass(); } \
		FORCEINLINE T* GetFromCtx(const FFINExecutionContext& Ctx) { return Cast<T>(Ctx.GetObject()); } \
		FFINStaticGlobalRegisterFunc RegClass([](){ \
			UFINStaticReflectionSource::AddClass(T::StaticClass(), FFINStaticClassReg{TEXT(InternalName), FINRefTypeLocText("DisplayName", DisplayName), FINRefTypeLocText("Description", Description)}); \
		});
#define EndClass() };
#define TypeStructName(Type) FIN_StaticRef_ ## Type
#define _BeginStruct(Type, InternalName, DisplayName, Description, bConstructable) \
	namespace TypeStructName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(InternalName) ; \
		UScriptStruct* GetUType() { return TBaseStructure<T>::Get(); } \
		FORCEINLINE T* GetFromCtx(const FFINExecutionContext& Ctx) { return static_cast<T*>(Ctx.GetGeneric()); } \
		FFINStaticGlobalRegisterFunc RegStruct([](){ \
			UFINStaticReflectionSource::AddStruct(GetUType(), FFINStaticStructReg(TEXT(InternalName), FINRefTypeLocText("DisplayName", DisplayName), FINRefTypeLocText("Description", Description), bConstructable)); \
		});
#define BeginStruct(Type, InternalName, DisplayName, Description) _BeginStruct(Type, InternalName, DisplayName, Description, false)
#define BeginStructConstructable(Type, InternalName, DisplayName, Description) _BeginStruct(Type, InternalName, DisplayName, Description, true)
#define EndStruct() };
#define GetClassFunc [](){ return T::StaticClass(); }
#define FuncClassName(Prefix, Func) FIN_StaticRefFunc_ ## Prefix ## _ ## Func
#define FINRefFuncLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))

#define ParamCounterIncrease __ParamCounter__::next<__LINE__>()
#define BeginFuncRT(Prefix, InternalName, DisplayName, Description, Varargs, FuncType, Runtime) \
	namespace FuncClassName(Prefix, InternalName) { \
		struct __ParamCounter__ : ::fameta::counter<__ParamCounter__> {}; \
		constexpr int F = __COUNTER__; \
		constexpr auto FName = TEXT(#InternalName) ; \
		void Execute(const FFINExecutionContext& Ctx, TArray<FINAny>& Params); \
		FFINStaticGlobalRegisterFunc RegClass([](){ \
			UFINStaticReflectionSource::AddFunction(GetUType(), F, FFINStaticFuncReg{TEXT(#InternalName), FINRefFuncLocText("DisplayName", DisplayName), FINRefFuncLocText("Description", Description), Varargs, &Execute, Runtime, FuncType}); \
			TArray<FINAny> Params; \
			Execute(FINTrace(nullptr), Params); \
		}); \
		void Execute(const FFINExecutionContext& Ctx, TArray<FINAny>& Params) { \
			static bool _bGotReg = false;
#define GET_MACRO(_0, VAL,...) VAL
#define BeginFunc(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, false, 0, GET_MACRO(0 , ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginOp(InternalName, OperatorNum, DisplayName, Description, ...) BeginFuncRT(Member, InternalName ## _ ## OperatorNum, DisplayName, Description, false, 0, GET_MACRO(0 , ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginFuncVA(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, true, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginClassFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Class, InternalName, DisplayName, Description, VA, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define BeginStaticFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Static, InternalName, DisplayName, Description, VA, 2, GET_MACRO(0, ##__VA_ARGS__, 1) )
#define FuncBody() \
			if (self && _bGotReg) {
#define EndFunc() \
			else if (!_bGotReg) _bGotReg = true; \
			} \
		} \
	};
#define PropClassName(Prefix, Prop) FIN_StaticRefProp_ ## Prefix ## _ ## Prop
#define FINRefPropLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(PName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginPropRT(Prefix, Type, InternalName, DisplayName, Description, PropType, Runtime) \
	namespace PropClassName(Prefix, InternalName) { \
		const int P = __COUNTER__; \
		constexpr auto PName = TEXT(#InternalName) ; \
		using PT = Type; \
		FINAny Get(const FFINExecutionContext& Ctx); \
		FFINStaticGlobalRegisterFunc RegProp([](){ \
			UFINStaticReflectionSource::AddProp(GetUType(), P, FFINStaticPropReg{TEXT(#InternalName), FINRefPropLocText("DisplayName", DisplayName), FINRefPropLocText("Description", Description), &Get, Runtime, PropType, &PT::PropConstructor}); \
		}); \
		FINAny Get(const FFINExecutionContext& Ctx) {
#define BeginProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Member, Type, InternalName, DisplayName, Description, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	T* self = GetFromCtx(Ctx);
#define BeginClassProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Class, Type, InternalName, DisplayName, Description, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define Return \
		return (FINAny)
#define PropSet() \
		} \
		void Set(const FFINExecutionContext& Ctx, const FINAny& Val); \
		FFINStaticGlobalRegisterFunc RegPropSet([](){ \
			UFINStaticReflectionSource::AddPropSetter(GetUType(), P, &Set); \
		}); \
		void Set(const FFINExecutionContext& Ctx, const FINAny& AnyVal) { \
			T* self = GetFromCtx(Ctx); \
			PT::CppType Val = PT::Get(AnyVal);
#define EndProp() \
		} \
	};

#define FINRefParamLocText(ParamName, KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define InVal(Type, InternalName, DisplayName, Description) \
	Type::CppType InternalName = Type::CppType(); \
	{ \
		constexpr auto __ParamCount__ = ParamCounterIncrease; \
		if (!_bGotReg) { \
			UFINStaticReflectionSource::AddFuncParam(GetUType(), F, __ParamCount__, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 0, &Type::PropConstructor}); \
		} \
		else InternalName = Type::Get(Params[__ParamCount__]); \
	}
#define OutVal(Type, InternalName, DisplayName, Description) \
	FINAny& InternalName = *(FINAny*)nullptr; \
	{ \
		constexpr auto __ParamCount__ = ParamCounterIncrease; \
		if (!_bGotReg) { \
			UFINStaticReflectionSource::AddFuncParam(GetUType(), F, __ParamCount__, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 1, &Type::PropConstructor}); \
		} \
		else InternalName = Params[__ParamCount__]; \
	}
#define RetVal(Type, InternalName, DisplayName, Description) \
	FINAny& InternalName = *(FINAny*)nullptr; \
	{ \
		constexpr auto __ParamCount__ = ParamCounterIncrease; \
		if (!_bGotReg) { \
			UFINStaticReflectionSource::AddFuncParam(GetUType(), F, __ParamCount__, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 3, &Type::PropConstructor}); \
		} \
		else InternalName = Params[__ParamCount__]; \
	}

#define FINRefSignalLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define FINRefSignalParamLocText(ParamName, KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define SignalClassName(Prop) FIN_StaticRefSignal_ ## Prop
#define BeginSignal(InternalName, DisplayName, Description, ...) \
	namespace SignalClassName(InternalName) { \
		const int S = __COUNTER__; \
		struct __ParamCounter__ : ::fameta::counter<__ParamCounter__> {}; \
		constexpr auto SName = TEXT(#InternalName) ; \
		FFINStaticGlobalRegisterFunc RegSignal([](){ \
			UFINStaticReflectionSource::AddSignal(GetUType(), S, FFINStaticSignalReg{TEXT(#InternalName), FINRefSignalLocText("DisplayName", DisplayName), FINRefSignalLocText("Description", Description), GET_MACRO(0, ##__VA_ARGS__, false)});
#define SignalParam(Type, InternalName, DisplayName, Description) \
			{ \
				constexpr auto __ParamCount__ = ParamCounterIncrease; \
				UFINStaticReflectionSource::AddSignalParam(GetUType(), S, __ParamCount__, FFINStaticSignalParamReg{TEXT(#InternalName), FINRefSignalParamLocText(#InternalName, "DisplayName", DisplayName), FINRefSignalParamLocText(#InternalName, "Description", Description), &Type::PropConstructor}); \
			}
#define EndSignal() \
		}); \
	};

#define Hook(HookClass) \
	FFINStaticGlobalRegisterFunc Hook([](){ \
		AFINHookSubsystem::RegisterHook(GetUType(), HookClass::StaticClass()); \
	});

#define TFS(Str) FText::FromString( Str )
