#pragma once

#include "FINArrayProperty.h"
#include "FINBoolProperty.h"
#include "FINClassProperty.h"
#include "FINFloatProperty.h"
#include "FINIntProperty.h"
#include "FINObjectProperty.h"
#include "FINReflectionSource.h"
#include "FINStrProperty.h"
#include "FINStructProperty.h"
#include "FINTraceProperty.h"

#include "FGBuildableDoor.h"
#include "FGRailroadSignalBlock.h"

#include "FINStaticReflectionSource.generated.h"

struct FICSITNETWORKS_API FFINStaticFuncParamReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	int ParamType;
	UFINProperty*(*PropConstructor)(UObject*);
};

struct FICSITNETWORKS_API FFINStaticFuncReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool VarArgs;
	TFunction<void(const FFINExecutionContext&, TArray<FINAny>&)> Function;
	int Runtime;
	int FuncType;
	TMap<int, FFINStaticFuncParamReg> Parameters;
};

struct FICSITNETWORKS_API FFINStaticPropReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TFunction<FINAny(const FFINExecutionContext&)>Get;
	int Runtime;
	int PropType;
	UFINProperty*(*PropConstructor)(UObject*);
	TFunction<void(const FFINExecutionContext&, const FINAny&)> Set;
};

struct FICSITNETWORKS_API FFINStaticSignalParamReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	UFINProperty*(*PropConstructor)(UObject*);
};

struct FICSITNETWORKS_API FFINStaticSignalReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool bIsVarArgs;
	TMap<int, FFINStaticSignalParamReg> Parameters;
};

struct FICSITNETWORKS_API FFINStaticClassReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TMap<int, FFINStaticFuncReg> Functions;
	TMap<int, FFINStaticPropReg> Properties;
	TMap<int, FFINStaticSignalReg> Signals;
};

struct FICSITNETWORKS_API FFINStaticStructReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool bConstructable;
	TMap<int, FFINStaticFuncReg> Functions;
	TMap<int, FFINStaticPropReg> Properties;

	FFINStaticStructReg() = default;
	FFINStaticStructReg(const FString& InternalName, const FText& DisplayName, const FText& Description, bool bConstructable) : InternalName(InternalName), DisplayName(DisplayName), Description(Description), bConstructable(bConstructable) {}
};

UCLASS()
class FICSITNETWORKS_API UFINStaticReflectionSource : public UFINReflectionSource {
	GENERATED_BODY()
	
	static TMap<UClass*, FFINStaticClassReg> Classes;
	static TMap<UScriptStruct*, FFINStaticStructReg> Structs;
	
public:
	static void AddClass(UClass* Class, const FFINStaticClassReg& ClassReg) {
		Classes.FindOrAdd(Class) = ClassReg;
	}
	static void AddStruct(UScriptStruct* Struct, const FFINStaticStructReg& StructReg) {
		Structs.Add(Struct, StructReg);
	}
	static void AddFunction(UClass* Class, int FuncID, const FFINStaticFuncReg& FuncReg) {
		Classes.Find(Class)->Functions.FindOrAdd(FuncID) = FuncReg;
	}
	static void AddFunction(UScriptStruct* Struct, int FuncID, const FFINStaticFuncReg& FuncReg) {
		FFINStaticStructReg* StructReg = Structs.Find(Struct);
		FFINStaticFuncReg& Reg = StructReg->Functions.FindOrAdd(FuncID, FuncReg);
		Reg.InternalName.RemoveFromEnd(TEXT("_0"));
	}
	static void AddFuncParam(UClass* Class, int FuncID, int ParamPos, const FFINStaticFuncParamReg& ParamReg) {
		Classes.FindOrAdd(Class).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddFuncParam(UScriptStruct* Struct, int FuncID, int ParamPos, const FFINStaticFuncParamReg& ParamReg) {
		Structs.FindOrAdd(Struct).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddProp(UClass* Class, int PropID, const FFINStaticPropReg& PropReg) {
		Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddProp(UScriptStruct* Struct, int PropID, const FFINStaticPropReg& PropReg) {
		Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddPropSetter(UClass* Class, int PropID, const TFunction<void(const FFINExecutionContext&, const FINAny&)>& Set) {
		FFINStaticPropReg& Reg = Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}
	static void AddPropSetter(UScriptStruct* Struct, int PropID, const TFunction<void(const FFINExecutionContext&, const FINAny&)>& Set) {
		FFINStaticPropReg& Reg = Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}
	static void AddSignal(UClass* Class, int SignalID, const FFINStaticSignalReg& SignalReg) {
		Classes.FindOrAdd(Class).Signals.FindOrAdd(SignalID) = SignalReg;
	}
	static void AddSignalParam(UClass* Class, int SignalID, int ParamPos, const FFINStaticSignalParamReg& SignalParamReg) {
		Classes.FindOrAdd(Class).Signals.FindOrAdd(SignalID).Parameters.FindOrAdd(ParamPos) = SignalParamReg;
	}

	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const override;
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const override;
	virtual void FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const override;
	// End UFINReflectionSource
};

struct RInt {
	typedef FINInt CppType;
	static FINInt Get(const FFINAnyNetworkValue& Any) { return Any.GetInt(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINIntProperty>(Outer);
	}
};

struct RFloat {
	typedef FINFloat CppType;
	static FINFloat Get(const FINAny& Any) { return Any.GetFloat(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINFloatProperty>(Outer);
	}
};

struct RBool {
	typedef FINBool CppType;
	static FINBool Get(const FINAny& Any) { return Any.GetBool(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINBoolProperty>(Outer);
	}
};

struct RString {
	typedef FINStr CppType;
	static FINStr Get(const FINAny& Any) { return Any.GetString(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINStrProperty>(Outer);
	}
};

template<typename T>
struct RClass {
	typedef FINClass CppType;
	static FINClass Get(const FINAny& Any) { return Any.GetClass(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINClassProperty* Prop = NewObject<UFINClassProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RObject {
	typedef TWeakObjectPtr<T> CppType;
	static CppType Get(const FINAny& Any) { return CppType(Cast<T>(Any.GetObj().Get())); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINObjectProperty* Prop = NewObject<UFINObjectProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RTrace {
	typedef FINTrace CppType;
	static FINTrace Get(const FINAny& Any) { return Any.GetTrace(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINTraceProperty* Prop = NewObject<UFINTraceProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};
 
template<typename T>
struct RStruct {
	typedef T CppType;
	static T Get(const FINAny& Any) { return Any.GetStruct().Get<T>(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINStructProperty* FINProp = NewObject<UFINStructProperty>(Outer);
		FINProp->Struct = TBaseStructure<T>::Get();
		return FINProp;
	}
};

template<typename T>
struct RArray {
	typedef FINArray CppType;
	static FINArray Get(const FINAny& Any) { return Any.GetArray(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINArrayProperty* FINProp = NewObject<UFINArrayProperty>(Outer);
		FINProp->InnerType = T::PropConstructor(FINProp);
		return FINProp;
	}
};

class FStaticReflectionSourceHelper {
public:
	
	static FINInt AFGBuildableDoor_GetConfig(AFGBuildableDoor *Door);
	static void AFGBuildableDoor_Update(AFGBuildableDoor* Door, EDoorConfiguration Config);

	static TArray<TWeakObjectPtr<AFGRailroadVehicle>> FFGRailroadSignalBlock_GetOccupiedBy(const FFGRailroadSignalBlock& Block);
	static TArray<TSharedPtr<FFGRailroadBlockReservation>> FFGRailroadSignalBlock_GetQueuedReservations(const FFGRailroadSignalBlock& Block);
	static TArray<TSharedPtr<FFGRailroadBlockReservation>> FFGRailroadSignalBlock_GetApprovedReservations(const FFGRailroadSignalBlock& Block);
};