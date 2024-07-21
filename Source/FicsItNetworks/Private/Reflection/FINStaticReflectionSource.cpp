#include "Reflection/FINStaticReflectionSource.h"
#include "Reflection/FINReflection.h"

TMap<UClass*, FFINStaticClassReg> UFINStaticReflectionSource::Classes;
TMap<UScriptStruct*, FFINStaticStructReg> UFINStaticReflectionSource::Structs;

bool UFINStaticReflectionSource::ProvidesRequirements(UClass *Class) const {
	return Classes.Contains(Class);
}

bool UFINStaticReflectionSource::ProvidesRequirements(UScriptStruct *Struct) const {
	return Structs.Contains(Struct);
}

void UFINStaticReflectionSource::FillData(FFINReflection *Ref, UFINClass *ToFillClass, UClass *Class) const {
	const FFINStaticClassReg *ClassReg = Classes.Find(Class);
	if (!ClassReg)
		return;
	ToFillClass->InternalName = ClassReg->InternalName;
	ToFillClass->DisplayName = ClassReg->DisplayName;
	ToFillClass->Description = ClassReg->Description;
	ToFillClass->Parent = Ref->FindClass(Class->GetSuperClass());
	ToFillClass->StructFlags |= FIN_Struct_StaticSource;
	if (ToFillClass->Parent == ToFillClass)
		ToFillClass->Parent = nullptr;

	for (const TPair<int, FFINStaticFuncReg> &KVFunc : ClassReg->Functions) {
		const FFINStaticFuncReg &Func = KVFunc.Value;
		UFINFunction *FINFunc = NewObject<UFINFunction>(ToFillClass);
		FINFunc->InternalName = Func.InternalName;
		FINFunc->DisplayName = Func.DisplayName;
		FINFunc->Description = Func.Description;
		FINFunc->FunctionFlags |= FIN_Func_StaticSource;
		if (Func.VarArgs)
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_VarArgs;
		switch (Func.Runtime) {
			case 0:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Sync;
				break;
			case 1:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Parallel;
				break;
			case 2:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Async;
				break;
			default:
				break;
		}
		switch (Func.FuncType) {
			case 1:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_ClassFunc;
				break;
			case 2:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_StaticFunc;
				break;
			default:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_MemberFunc;
				break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticFuncParamReg &Param = Func.Parameters[Pos];
			UFINProperty *FINProp = Param.PropConstructor(FINFunc);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param | FIN_Prop_StaticSource;
			switch (Param.ParamType) {
				case 2:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
				case 1:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
					break;
				default:
					break;
			}
			FINFunc->Parameters.Add(FINProp);
		}

		auto NFunc = Func.Function;
		FINFunc->NativeFunction = [Func](const FFINExecutionContext &Ctx,
		                                 const TArray<FINAny> &InValues) -> TArray<FINAny> {
			TArray<FINAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0)
				for (int i = 0; i <= Pos[Pos.Num() - 1]; ++i) {
					const FFINStaticFuncParamReg *Reg = Func.Parameters.Find(i);
					if (Reg && Reg->ParamType == 0) {
						Parameters.Add(InValues[j++]);
					} else {
						Parameters.Add(FINAny());
					}
				}
			for (; j < InValues.Num(); j++)
				Parameters.Add(InValues[j]);
			Func.Function(Ctx, Parameters);

			TArray<FINAny> OutValues;
			if (Pos.Num() > 0)
				for (int i = 0; i <= Pos[Pos.Num() - 1]; ++i) {
					const FFINStaticFuncParamReg *Reg = Func.Parameters.Find(i);
					if (Reg && Reg->ParamType > 0) {
						OutValues.Add(Parameters[i]);
						j++;
					}
				}
			for (; j < Parameters.Num();)
				OutValues.Add(Parameters[j++]);
			return OutValues;
		};

		ToFillClass->Functions.Add(FINFunc);
	}

	for (const TPair<int, FFINStaticPropReg> &KVProp : ClassReg->Properties) {
		const FFINStaticPropReg &Prop = KVProp.Value;
		UFINProperty *FINProp = Prop.PropConstructor(ToFillClass);
		FINProp->InternalName = Prop.InternalName;
		FINProp->DisplayName = Prop.DisplayName;
		FINProp->Description = Prop.Description;
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib | FIN_Prop_StaticSource;
		if (UFINFuncProperty *FINFuncProp = Cast<UFINFuncProperty>(FINProp)) {
			FINFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set)
				FINFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else
				FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
			case 0:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Sync;
				break;
			case 1:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Parallel;
				break;
			case 2:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Async;
				break;
			default:
				break;
		}
		switch (Prop.PropType) {
			case 1:
				FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ClassProp;
				break;
			default:
				break;
		}
		ToFillClass->Properties.Add(FINProp);
	}

	for (const TPair<int, FFINStaticSignalReg> &KVSignal : ClassReg->Signals) {
		const FFINStaticSignalReg &Signal = KVSignal.Value;
		UFINSignal *FINSignal = NewObject<UFINSignal>(ToFillClass);
		FINSignal->InternalName = Signal.InternalName;
		FINSignal->DisplayName = Signal.DisplayName;
		FINSignal->Description = Signal.Description;
		FINSignal->bIsVarArgs = Signal.bIsVarArgs;
		FINSignal->SignalFlags = FIN_Signal_StaticSource;

		TArray<int> ParamPos;
		Signal.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticSignalParamReg &Param = Signal.Parameters[Pos];
			UFINProperty *FINProp = Param.PropConstructor(FINSignal);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
			FINSignal->Parameters.Add(FINProp);
		}
		ToFillClass->Signals.Add(FINSignal);
	}
}

void UFINStaticReflectionSource::FillData(FFINReflection *Ref, UFINStruct *ToFillStruct, UScriptStruct *Struct) const {
	const FFINStaticStructReg *StructReg = Structs.Find(Struct);
	if (!StructReg)
		return;
	ToFillStruct->InternalName = StructReg->InternalName;
	ToFillStruct->DisplayName = StructReg->DisplayName;
	ToFillStruct->Description = StructReg->Description;
	ToFillStruct->Parent = Ref->FindStruct(Cast<UScriptStruct>(Struct->GetSuperStruct()));
	ToFillStruct->StructFlags |= FIN_Struct_StaticSource;
	if (StructReg->bConstructable)
		ToFillStruct->StructFlags |= FIN_Struct_Constructable;
	if (ToFillStruct->Parent == ToFillStruct)
		ToFillStruct->Parent = nullptr;

	for (const TPair<int, FFINStaticFuncReg> &KVFunc : StructReg->Functions) {
		const FFINStaticFuncReg &Func = KVFunc.Value;
		UFINFunction *FINFunc = NewObject<UFINFunction>(ToFillStruct);
		FINFunc->InternalName = Func.InternalName;
		FINFunc->DisplayName = Func.DisplayName;
		FINFunc->Description = Func.Description;
		FINFunc->FunctionFlags |= FIN_Func_StaticSource;
		if (Func.VarArgs)
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_VarArgs;
		switch (Func.Runtime) {
			case 0:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Sync;
				break;
			case 1:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Parallel;
				break;
			case 2:
				FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Async;
				break;
			default:
				break;
		}
		switch (Func.FuncType) {
			case 1:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_ClassFunc;
				break;
			case 2:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_StaticFunc;
				break;
			default:
				FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_MemberFunc;
				break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticFuncParamReg &Param = Func.Parameters[Pos];
			UFINProperty *FINProp = Param.PropConstructor(FINFunc);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param | FIN_Prop_StaticSource;
			switch (Param.ParamType) {
				case 2:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
				case 1:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
					break;
				default:
					break;
			}
			FINFunc->Parameters.Add(FINProp);
		}

		auto NFunc = Func.Function;
		FINFunc->NativeFunction = [Func](const FFINExecutionContext &Ctx,
		                                 const TArray<FINAny> &Params) -> TArray<FINAny> {
			TArray<FINAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0)
				for (int i = 0; i <= Pos[Pos.Num() - 1]; ++i) {
					const FFINStaticFuncParamReg *Reg = Func.Parameters.Find(i);
					if (Reg && Reg->ParamType == 0) {
						Parameters.Add(Params[j++]);
					} else {
						Parameters.Add(FINAny());
					}
				}
			Func.Function(Ctx, Parameters);

			TArray<FINAny> OutValues;
			if (Pos.Num() > 0)
				for (int i = 0; i <= Pos[Pos.Num() - 1]; ++i) {
					const FFINStaticFuncParamReg *Reg = Func.Parameters.Find(i);
					if (Reg && Reg->ParamType > 0) {
						OutValues.Add(Parameters[i]);
					}
				}
			return OutValues;
		};

		ToFillStruct->Functions.Add(FINFunc);
	}

	for (const TPair<int, FFINStaticPropReg> &KVProp : StructReg->Properties) {
		const FFINStaticPropReg &Prop = KVProp.Value;
		UFINProperty *FINProp = Prop.PropConstructor(ToFillStruct);
		FINProp->InternalName = Prop.InternalName;
		FINProp->DisplayName = Prop.DisplayName;
		FINProp->Description = Prop.Description;
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib | FIN_Prop_StaticSource;
		if (UFINFuncProperty *FINFuncProp = Cast<UFINFuncProperty>(FINProp)) {
			FINFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set)
				FINFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else
				FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
			case 0:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Sync;
				break;
			case 1:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Parallel;
				break;
			case 2:
				FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Async;
				break;
			default:
				break;
		}
		switch (Prop.PropType) {
			case 1:
				FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ClassProp;
				break;
			default:
				break;
		}
		ToFillStruct->Properties.Add(FINProp);
	}
}

FINInt FStaticReflectionSourceHelper::AFGBuildableDoor_GetConfig(AFGBuildableDoor *Door) {
	return (FINInt)Door->GetmDoorConfiguration();
}
void FStaticReflectionSourceHelper::AFGBuildableDoor_Update(AFGBuildableDoor *Door, EDoorConfiguration Config) {
	Door->SetmDoorConfiguration(Config);
	Door->OnRep_DoorConfiguration();
}

TArray<TWeakObjectPtr<AFGRailroadVehicle>> FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetOccupiedBy(
	const FFGRailroadSignalBlock &Block) {
	return Block.mOccupiedBy;
}

TArray<TSharedPtr<FFGRailroadBlockReservation>>
FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetQueuedReservations(
	const FFGRailroadSignalBlock &Block) {
	return Block.mQueuedReservations;
}

TArray<TSharedPtr<FFGRailroadBlockReservation>>
FStaticReflectionSourceHelper::FFGRailroadSignalBlock_GetApprovedReservations(
	const FFGRailroadSignalBlock &Block) {
	return Block.mApprovedReservations;
}
