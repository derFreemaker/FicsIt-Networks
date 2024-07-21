#include "Reflection/FINReflection.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINBoolProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINFloatProperty.h"
#include "Reflection/FINFuncProperty.h"
#include "Reflection/FINIntProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINStaticReflectionSource.h"
#include "Reflection/FINStrProperty.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "Reflection/FINUReflectionSource.h"
#include "FicsItNetworksModule.h"
#include "AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

UFINClass* UFINReflection::FindClass(UClass* Clazz, bool bRecursive) {
	return FFINReflection::Get()->FindClass(Clazz, bRecursive);
}

UFINStruct* UFINReflection::FindStruct(UScriptStruct* Struct, bool bRecursive) {
	return FFINReflection::Get()->FindStruct(Struct, bRecursive);
}

FFINReflection* FFINReflection::Get() {
	static FFINReflection* Self = nullptr;
	if (!Self) Self = new FFINReflection();
	return Self;
}

void FFINReflection::PopulateSources() {
	Sources.Add(GetDefault<UFINUReflectionSource>());
	Sources.Add(GetDefault<UFINStaticReflectionSource>());
}

void FFINReflection::LoadAllTypes() {
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

#if WITH_EDITOR
	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/FicsItNetworks/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan, true);

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Add("/");
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UBlueprintGeneratedClass::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UClass::StaticClass()->GetClassPathName());
	AssetRegistry.GetAssets(Filter, AssetData);

	for (const FAssetData& Asset : AssetData) {
		FString Path = Asset.GetObjectPathString();
		if (!Path.EndsWith("_C")) Path += "_C";
		UClass* Class = LoadClass<UObject>(NULL, *Path);
		if (!Class) {
			Class = LoadClass<UObject>(NULL, *Path);
		}
		if (!Class) continue;
		FindClass(Class);
	}
#else
	TArray<FTopLevelAssetPath> BaseNames;
	BaseNames.Add(UObject::StaticClass()->GetClassPathName());
	TSet<FTopLevelAssetPath> Excluded;
	TSet<FTopLevelAssetPath> DerivedNames;
	AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);

	for (const FTopLevelAssetPath& ClassName : DerivedNames) {
		UClass* Class = TAssetSubclassOf(FSoftObjectPath(ClassName)).LoadSynchronous();
		if (Class->GetClassFlags() & (CLASS_Abstract | CLASS_Hidden) || Class->GetName().StartsWith("SKEL_")) continue;
		FindClass(Class);
	}
#endif

	for (TObjectIterator<UClass> Class; Class; ++Class) {
		if (!Class->GetName().StartsWith("SKEL_") && !Class->GetName().StartsWith("REINST_")) FindClass(*Class);
	}

	for (TObjectIterator<UScriptStruct> Struct; Struct; ++Struct) {
		if (!Struct->GetName().StartsWith("SKEL_") && !Struct->GetName().StartsWith("REINST_")) FindStruct(*Struct);
	}
}

UFINClass* FFINReflection::FindClass(UClass* Clazz, bool bRecursive, bool bTryToReflect) {
	if (!Clazz) return nullptr;
	do {
		// Find class in cache and retrun if found
		{
			UFINClass** Class = Classes.Find(Clazz);
			if (Class) return *Class;
		}

		// try to load this exact class into cache and return it
		if (bTryToReflect) {
			UFINClass* Class = nullptr;
			for (const UFINReflectionSource* Source : Sources) {
				if (Source->ProvidesRequirements(Clazz)) {
					Class = NewObject<UFINClass>(Clazz);
					break;
				}
			}
			if (Class) {
				Clazz->AddToRoot();
				Class->AddToRoot();
				Classes.Add(Clazz, Class);
				ClassesReversed.Add(Class, Clazz);
				for (const UFINReflectionSource* Source : Sources) {
					Source->FillData(this, Class, Clazz);
				}
				ClassNames.Add(Class->GetInternalName(), Class);
				return Class;
			}
		}

		// go to the next super class
		if (Clazz == UObject::StaticClass()) Clazz = nullptr;
		else Clazz = Clazz->GetSuperClass();
	} while (Clazz && bRecursive);
	return nullptr;
}

UFINClass* FFINReflection::FindClass(const FString& ClassName) const {
	UFINClass* const* Class = ClassNames.Find(ClassName);
	if (Class) return *Class;
	else return nullptr;
}

UClass* FFINReflection::FindUClass(UFINClass* Class) const {
	UClass* const* UClass = ClassesReversed.Find(Class);
	if (UClass) return *UClass;
	else return nullptr;
}

UFINStruct* FFINReflection::FindStruct(UScriptStruct* Struct, bool bRecursive, bool bTryToReflect) {
	if (!Struct) return nullptr;
	do {
		// Find class in cache and retrun if found
		{
			UFINStruct** FINStruct = Structs.Find(Struct);
			if (FINStruct) return *FINStruct;
		}

		// try to load this exact class into cache and return it
		if (bTryToReflect) {
			UFINStruct* FINStruct = nullptr;
			for (const UFINReflectionSource* Source : Sources) {
				if (Source->ProvidesRequirements(Struct)) {
					FINStruct = NewObject<UFINStruct>(Struct);
					break;
				}
			}
			if (FINStruct) {
				Struct->AddToRoot();
				FINStruct->AddToRoot();
				Structs.Add(Struct, FINStruct);
				StructsReversed.Add(FINStruct, Struct);
				for (const UFINReflectionSource* Source : Sources) {
					Source->FillData(this, FINStruct, Struct);
				}
				StructNames.Add(FINStruct->GetInternalName(), FINStruct);
				return FINStruct;
			}
		}

		// go to the next super class
		Struct = Cast<UScriptStruct>(Struct->GetSuperStruct());
	} while (Struct && bRecursive);
	return nullptr;
}

UFINStruct* FFINReflection::FindStruct(const FString& StructName) const {
	UFINStruct* const* Struct = StructNames.Find(StructName);
	if (Struct) return *Struct;
	else return nullptr;
}

UScriptStruct* FFINReflection::FindScriptStruct(UFINStruct* Struct) const {
	UScriptStruct* const* ScriptStruct = StructsReversed.Find(Struct);
	if (ScriptStruct) return *ScriptStruct;
	else return nullptr;
}

void PrintProperty(FString Prefix, UFINProperty* Property) {
	FString Log = Prefix;
	if (Property->GetPropertyFlags() & FIN_Prop_Attrib) Log += "Attrib";
	else if (Property->GetPropertyFlags() & FIN_Prop_Param) Log += "Param";
	Log += " " + Property->GetInternalName() + " '" + Property->GetDisplayName().ToString() + "' Desc:'" + Property->GetDescription().ToString() + "' " + FString::FromInt(Property->GetType()) + " '" + Property->GetClass()->GetName() + "'";
	if (Property->GetPropertyFlags() & FIN_Prop_OutParam) Log += " Out";
	if (Property->GetPropertyFlags() & FIN_Prop_RetVal) Log += " RetVal";
	if (Property->GetPropertyFlags() & FIN_Prop_ReadOnly) Log += " ReadOnly";
	if (UFINFuncProperty* FINFuncProp = Cast<UFINFuncProperty>(Property)) {
		if (FINFuncProp->GetterFunc.Function) Log += " UFuncGetter";
		if (FINFuncProp->SetterFunc.Function) Log += " UFuncSetter";
		if ((bool)FINFuncProp->GetterFunc.GetterFunc) Log += " FuncGetter";
		if ((bool)FINFuncProp->SetterFunc.SetterFunc) Log += " FuncSetter";
	}
	if (UFINStructProperty* FINStructProp = Cast<UFINStructProperty>(Property)) {
		if (FINStructProp->Struct) Log += " " + FINStructProp->Struct->GetName();
	}
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Sync) Log += " Sync";
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Parallel) Log += " Parallel";
	if (Property->GetPropertyFlags() & FIN_Prop_RT_Async) Log += " Async";
	if (Property->GetPropertyFlags() & FIN_Prop_ClassProp) Log += " Class";
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s"), *Log);
}

void PrintFunction(FString Prefix, UFINFunction* Function) {
	FString Log = Prefix;
	Log += "Function: " + Function->GetInternalName();
	Log += " '" + Function->GetDisplayName().ToString() + "'";
	Log += " Desc:'" + Function->GetDescription().ToString() + "'";
	if (Function->GetFunctionFlags() & FIN_Func_VarArgs) Log += " Varargs";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Sync) Log += " Sync";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Parallel) Log += " Parallel";
	if (Function->GetFunctionFlags() & FIN_Func_RT_Async) Log += " Async";
	if (Function->GetFunctionFlags() & FIN_Func_ClassFunc) Log += " Class";
	if (Function->GetFunctionFlags() & FIN_Func_StaticFunc) Log += " Static";
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s"), *Log);
	Prefix += " ";
	for (UFINProperty* Param : Function->GetParameters()) {
		PrintProperty(Prefix, Param);
	}
}

void PrintSignal(FString Prefix, UFINSignal* Signal) {
	FString Log = Prefix;
	Log += "Signal: " + Signal->GetInternalName();
	Log += " '" + Signal->GetDisplayName().ToString() + "'";
	Log += " Desc:'" + Signal->GetDescription().ToString() + "'";
	UE_LOG(LogFicsItNetworks, Display, TEXT("%s"), *Log);
	Prefix += " ";
	for (UFINProperty* Param : Signal->GetParameters()) {
		PrintProperty(Prefix, Param);
	}
}

void FFINReflection::PrintReflection() {
	for (TPair<UClass*, UFINClass*> Class : Classes) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Class: %s '%s' Desc:'%s'"), *Class.Value->GetInternalName(), *Class.Value->GetDisplayName().ToString(), *Class.Value->GetDescription().ToString());
		for (UFINFunction* Function : Class.Value->GetFunctions()) {
			PrintFunction(" ", Function);
		}
		for (UFINProperty* Prop : Class.Value->GetProperties()) {
			PrintProperty(" ", Prop);
		}
		for (UFINSignal* Signal : Class.Value->GetSignals()) {
			PrintSignal(" ", Signal);
		}
	}

	for (TPair<UScriptStruct*, UFINStruct*> FINStruct : Structs) {
		FString Log = "Struct:";
		Log += " " + FINStruct.Value->GetInternalName();
		Log += " '" + FINStruct.Value->GetDisplayName().ToString() + "'";
		Log += " Desc:'" + FINStruct.Value->GetDescription().ToString() + "'";
		for (UFINFunction* Function : FINStruct.Value->GetFunctions()) {
			PrintFunction(" ", Function);
		}
		for (UFINProperty* Prop : FINStruct.Value->GetProperties()) {
			PrintProperty(" ", Prop);
		}
		UE_LOG(LogFicsItNetworks, Display, TEXT("Class: %s"), *Log);
	}
}

UFINProperty* FINCreateFINPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer) {
	UFINProperty* FINProp = nullptr;
	if (CastField<FStrProperty>(Property)) {
		UFINStrProperty* FINStrProp = NewObject<UFINStrProperty>(Outer);
		FINStrProp->Property = CastField<FStrProperty>(OverrideProperty);
		FINProp = FINStrProp;
	} else if (CastField<FIntProperty>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>(Outer);
		FINIntProp->Property = CastField<FIntProperty>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (CastField<FByteProperty>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>(Outer);
		FINIntProp->Property8 = CastField<FByteProperty>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (CastField<FInt64Property>(Property)) {
		UFINIntProperty* FINIntProp = NewObject<UFINIntProperty>(Outer);
		FINIntProp->Property64 = CastField<FInt64Property>(OverrideProperty);
		FINProp = FINIntProp;
	} else if (CastField<FFloatProperty>(Property)) {
		UFINFloatProperty* FINFloatProp = NewObject<UFINFloatProperty>(Outer);
		FINFloatProp->FloatProperty = CastField<FFloatProperty>(OverrideProperty);
		FINProp = FINFloatProp;
	} else if (CastField<FDoubleProperty>(Property)) {
		UFINFloatProperty* FINFloatProp = NewObject<UFINFloatProperty>(Outer);
		FINFloatProp->DoubleProperty = CastField<FDoubleProperty>(OverrideProperty);
		FINProp = FINFloatProp;
	} else if (CastField<FBoolProperty>(Property)) {
		UFINBoolProperty* FINBoolProp = NewObject<UFINBoolProperty>(Outer);
		FINBoolProp->Property = CastField<FBoolProperty>(OverrideProperty);
		FINProp = FINBoolProp;
	} else if (CastField<FClassProperty>(Property)) {
		UFINClassProperty* FINClassProp = NewObject<UFINClassProperty>(Outer);
		FINClassProp->Property = CastField<FClassProperty>(OverrideProperty);
		FINProp = FINClassProp;
	} else if (CastField<FObjectProperty>(Property)) {
		UFINObjectProperty* FINObjectProp = NewObject<UFINObjectProperty>(Outer);
		FINObjectProp->Property = CastField<FObjectProperty>(OverrideProperty);
		FINProp = FINObjectProp;
	} else  if (CastField<FStructProperty>(Property)) {
		FStructProperty* StructProp = CastField<FStructProperty>(OverrideProperty);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			UFINTraceProperty* FINTraceProp = NewObject<UFINTraceProperty>(Outer);
			FINTraceProp->Property = StructProp;
			FINProp = FINTraceProp;
		} else {
			UFINStructProperty* FINStructProp = NewObject<UFINStructProperty>(Outer);
			checkf(StructProp->Struct == FFINAnyNetworkValue::StaticStruct() || FFINReflection::Get()->FindStruct(StructProp->Struct) != nullptr, TEXT("Struct Property '%s' of reflection-base '%s' uses non-reflectable struct '%s'"), *Property->GetFullName(), *Outer->GetName(), *StructProp->Struct->GetName());
			FINStructProp->Property = StructProp;
			FINStructProp->Struct = StructProp->Struct;
			FINProp = FINStructProp;
		}
    } else if (CastField<FArrayProperty>(Property)) {
    	FArrayProperty* ArrayProperty = CastField<FArrayProperty>(OverrideProperty);
	    UFINArrayProperty* FINArrayProp = NewObject<UFINArrayProperty>(Outer);
    	FINArrayProp->Property = ArrayProperty;
    	FINArrayProp->InnerType = FINCreateFINPropertyFromFProperty(ArrayProperty->Inner, FINArrayProp);
    	FINProp = FINArrayProp;
    }
	check(FINProp != nullptr);
	if (Property->PropertyFlags & CPF_OutParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
	if (Property->PropertyFlags & CPF_ReturnParm) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
	return FINProp;
}
