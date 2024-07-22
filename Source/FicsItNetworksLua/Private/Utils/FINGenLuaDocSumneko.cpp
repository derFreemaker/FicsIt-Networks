﻿#include "Utils/FINGenLuaDocSumneko.h"

#include <functional>
#include <optional>
#include <FINLua/FINLuaModule.h>

#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "UObject/Package.h"

#pragma optimize("", off)
FString FINGenLuaSumnekoGetTypeName(const UFINBase *Base) {
	TArray<FString> SplitBasePackageName;
	Base->GetPackage()->GetName().ParseIntoArray(SplitBasePackageName, TEXT("/"), true);

	FString BasePackageName;
	for (auto &NamePart : SplitBasePackageName) {
		if (NamePart.Equals(TEXT("Script"))) {
			continue; // check next name part
		}

		// we only want the the first not filtered out name should be the mod name
		if (NamePart.Equals(TEXT("CoreUObject"))) {
			BasePackageName += TEXT("Engine.");
			break; // got one name part
		}

		// replace mod and Satisfactory name to make type names smaller
		if (NamePart.Equals(TEXT("FactoryGame"))
			|| NamePart.Equals(TEXT("Game"))) {
			BasePackageName += TEXT("Satis.");
			break; // got one name part
		} else if (NamePart.Equals(TEXT("FicsItNetworks"))
			|| NamePart.Equals(TEXT("FicsItNetworksLua"))) {
			BasePackageName += TEXT("FIN.");
			break; // got one name part
		}

		BasePackageName += NamePart + TEXT(".");
		break; // got one name part
	}

	return BasePackageName + Base->GetInternalName();
}

FString FINGenLuaSumnekoGetType(FFINReflection &Ref, const UFINProperty *Prop) {
	if (!Prop) {
		return "any";
	}

	switch (Prop->GetType()) {
		case FIN_NIL:
			return "nil";
		case FIN_BOOL:
			return "boolean";
		case FIN_INT:
		case FIN_FLOAT:
			return "number";
		case FIN_STR:
			return "string";
		case FIN_OBJ: {
			const UFINObjectProperty *ObjProp = Cast<UFINObjectProperty>(Prop);
			const UFINClass *Class = Ref.FindClass(ObjProp->GetSubclass());
			if (!Class)
				return "Engine.Object";
			return FINGenLuaSumnekoGetTypeName(Class);
		}
		case FIN_TRACE: {
			const UFINTraceProperty *TraceProp = Cast<UFINTraceProperty>(Prop);
			const UFINClass *Class = Ref.FindClass(TraceProp->GetSubclass());
			if (!Class)
				return "Engine.Object";
			return FINGenLuaSumnekoGetTypeName(Class);
		}
		case FIN_CLASS: {
			const UFINClassProperty *ClassProp = Cast<UFINClassProperty>(Prop);
			const UFINClass *Class = Ref.FindClass(ClassProp->GetSubclass());
			if (!Class)
				return "Engine.Object";
			return FINGenLuaSumnekoGetTypeName(Class);
		}
		case FIN_STRUCT: {
			const UFINStructProperty *StructProp = Cast<UFINStructProperty>(Prop);
			const UFINStruct *Struct = Ref.FindStruct(StructProp->GetInner());
			if (!Struct)
				return "any";
			return FINGenLuaSumnekoGetTypeName(Struct);
		}
		case FIN_ARRAY: {
			const UFINArrayProperty *ArrayProp = Cast<UFINArrayProperty>(Prop);
			return FINGenLuaSumnekoGetType(Ref, ArrayProp->GetInnerType()) + "[]";
		}
		default:
			return "any";
	}
}

FString FormatDescription(FString Description) {
	// assuming its only LF line endings
	Description.ReplaceCharInline('\n', ' ');
	return Description;
}

void FINGenLuaSumnekoDescription(FString &Str, FString Description) {
	Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

	FString Line;
	while (Description.Split(TEXT("\n"), &Line, &Description)) {
		Str.Append(TEXT("--- ") + Line + TEXT("<br>\n"));
	}
	Str.Append(TEXT("--- ") + Description + TEXT("\n"));
}

FString FINGenLuaSumnekoProperty(FFINReflection &Ref, const FString &Parent,
                                 const UFINProperty *Prop) {
	FString PropertyDocumentation = "\n";

	const EFINRepPropertyFlags PropFlags = Prop->GetPropertyFlags();
	FINGenLuaSumnekoDescription(PropertyDocumentation, TEXT("### Flags:"));

	if (PropFlags & FIN_Prop_RT_Sync) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}

	if (PropFlags & FIN_Prop_RT_Parallel) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}

	if (PropFlags & FIN_Prop_RT_Async) {
		FINGenLuaSumnekoDescription(PropertyDocumentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}

	if (PropFlags & FIN_Prop_ReadOnly) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Read Only - The value of this property can not be changed by code."));
	}

	PropertyDocumentation.Appendf(
		TEXT("---@type %s\n%s.%s = nil\n"), *FINGenLuaSumnekoGetType(Ref, Prop), *Parent, *Prop->GetInternalName());

	return PropertyDocumentation;
}

std::optional<FString> FINGenLuaSumnekoOperator(FFINReflection &Ref, const UFINFunction *Op) {
	FString OpName = Op->GetInternalName();
	FString OpTypeSumneko;

	if (OpName.Contains(TEXT("Add"))) {
		OpTypeSumneko = "add";
	} else if (OpName.Contains(TEXT("Sub"))) {
		OpTypeSumneko = "sub";
	} else if (OpName.Contains(TEXT("Mul"))) {
		OpTypeSumneko = "mul";
	} else if (OpName.Contains(TEXT("Div"))) {
		OpTypeSumneko = "div";
	} else if (OpName.Contains(TEXT("Mod"))) {
		OpTypeSumneko = "mod";
	} else if (OpName.Contains(TEXT("Pow"))) {
		OpTypeSumneko = "pow";
	} else if (OpName.Contains(TEXT("Neg"))) {
		OpTypeSumneko = "unm";
	} else if (OpName.Contains(TEXT("FDiv"))) {
		OpTypeSumneko = "idiv";
	} else if (OpName.Contains(TEXT("BitAND"))) {
		OpTypeSumneko = "band";
	} else if (OpName.Contains(TEXT("BitOR"))) {
		OpTypeSumneko = "bor";
	} else if (OpName.Contains(TEXT("BitXOR"))) {
		OpTypeSumneko = "bxor";
	} else if (OpName.Contains(TEXT("BitNot"))) {
		OpTypeSumneko = "bnot";
	} else if (OpName.Contains(TEXT("ShiftL"))) {
		OpTypeSumneko = "shl";
	} else if (OpName.Contains(TEXT("ShiftR"))) {
		OpTypeSumneko = "shr";
	} else if (OpName.Contains(TEXT("Concat"))) {
		OpTypeSumneko = "concat";
	} else if (OpName.Contains(TEXT("Len"))) {
		OpTypeSumneko = "len";
	} else if (OpName.Contains(TEXT("Call"))) {
		OpTypeSumneko = "call";
	}

	// are defined but not used and don't have a sunmeko equivalent
	// else if (OpName.Contains(TEXT("Equals"))) {
	// 	OpTypeSumneko = "";
	// } else if (OpName.Contains(TEXT("LessThan"))) {
	// 	OpTypeSumneko = "";
	// } else if (OpName.Contains(TEXT("LessOrEqualThen"))) {
	// 	OpTypeSumneko = "";
	// } else if (OpName.Contains(TEXT("Index"))) {
	// 	OpTypeSumneko = "";
	// } else if (OpName.Contains(TEXT("NewIndex"))) {
	// 	OpTypeSumneko = "";
	// }

	else {
		return std::nullopt;
	}

	FString OpParameter;
	FString OpReturn;
	for (const UFINProperty *Prop : Op->GetParameters()) {
		const EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();

		if (!(Flags & FIN_Prop_Param)) {
			continue; // skip prop
		}

		if (Flags & FIN_Prop_OutParam && OpReturn.Len() == 0) {
			OpReturn.Append(FINGenLuaSumnekoGetType(Ref, Prop));
			continue; // found return
		} else if (OpParameter.Len() == 0) {
			OpParameter.Append(FINGenLuaSumnekoGetType(Ref, Prop));
			continue; // found parameter
		}

		break; // already found parameter and return
	}

	return FString::Printf(TEXT("\n---@operator %s%s : %s"),
	                       *OpTypeSumneko,
	                       *(OpParameter.Len() > 0
		                         ? TEXT("(") + OpParameter + TEXT(")")
		                         : TEXT("")),
	                       *OpReturn);
}

FString FINGenLuaSumnekoFunction(FFINReflection &Ref, const FString &Parent,
                                 const UFINFunction *Func) {
	FString FunctionDocumentation = "\n";

	FINGenLuaSumnekoDescription(FunctionDocumentation, Func->GetDescription().ToString());

	const EFINFunctionFlags funcFlags = Func->GetFunctionFlags();
	FINGenLuaSumnekoDescription(FunctionDocumentation, TEXT("### Flags:"));

	if (funcFlags & FIN_Func_RT_Sync) {
		FINGenLuaSumnekoDescription(FunctionDocumentation,
		                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}

	if (funcFlags & FIN_Func_RT_Async) {
		FINGenLuaSumnekoDescription(FunctionDocumentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}
	
	if (funcFlags & FIN_Func_RT_Parallel) {
		FINGenLuaSumnekoDescription(FunctionDocumentation,
		                            TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}

	// by only sync we are assuming a future is getting returned
	bool OnlySync = funcFlags & FIN_Func_RT_Sync && !(funcFlags & FIN_Func_RT_Async) && !(funcFlags & FIN_Func_RT_Parallel);
	
	FString ParamDocumentation;
	FString ReturnDocumentation;
	FString FutureDocumentation;
	FString ParamList;
	for (const UFINProperty *Prop : Func->GetParameters()) {
		const EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();

		if (!(Flags & FIN_Prop_Param)) {
			continue; // skip prop
		}

		if (Flags & FIN_Prop_OutParam) {
			if (OnlySync) {
				FutureDocumentation.Appendf(TEXT("%s%s"),
					FutureDocumentation.Len() > 0
						? TEXT(", ")
						: TEXT("---@return FIN.Future<"),
					*FINGenLuaSumnekoGetType(Ref, Prop));
			}
			
			ReturnDocumentation.Appendf(TEXT("---@return %s %s %s\n"),
			                            *FINGenLuaSumnekoGetType(Ref, Prop),
			                            *Prop->GetInternalName(),
			                            *FormatDescription(Prop->GetDescription().ToString()));
		} else {
			ParamDocumentation.Appendf(
				TEXT("---@param %s %s %s\n"),
				*Prop->GetInternalName(),
				*FINGenLuaSumnekoGetType(Ref, Prop),
				*FormatDescription(Prop->GetDescription().ToString())
			);

			if (ParamList.Len() > 0) {
				ParamList.Append(", ");
			}

			ParamList.Append(Prop->GetInternalName());
		}
	}

	if (OnlySync && FutureDocumentation.Len() > 0) {
		FutureDocumentation.Append(TEXT("> future ## ! Only the Future gets returned the other returns are for documentation. !\n"));
	}
	
	if (funcFlags & FIN_Func_VarArgs) {
		ParamDocumentation.Append(TEXT("---@param ... any @additional arguments as described\n"));

		if (ParamList.Len() > 0) {
			ParamList.Append(", ");
		}

		ParamList.Append("...");
	}

	FunctionDocumentation.Append(
		FString::Printf(TEXT("%s%s%sfunction %s:%s(%s) end\n"),
		                *ParamDocumentation,
		                *FutureDocumentation,
		                *ReturnDocumentation,
		                *Parent,
		                *Func->GetInternalName(),
		                *ParamList));

	return FunctionDocumentation;
}

FString FINGenLuaSumnekoSignal(FFINReflection &Ref, const FString &Parent,
                               const UFINSignal *Signal) {
	FString SignalDocumentation = "\n";

	FINGenLuaSumnekoDescription(SignalDocumentation, Signal->GetDescription().ToString() + TEXT("\n"));

	FINGenLuaSumnekoDescription(SignalDocumentation, TEXT("### returns from event.pull:\n```"));

	SignalDocumentation.Append(TEXT("--- local signalName, component"));
	for (const UFINProperty *Prop : Signal->GetParameters()) {
		SignalDocumentation.Append(TEXT(", ") + Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")));
	}
	if (Signal->IsVarArgs()) {
		SignalDocumentation.Append(TEXT(", ..."));
	}
	SignalDocumentation.Append(TEXT(" = event.pull()\n--- ```\n"));

	FINGenLuaSumnekoDescription(SignalDocumentation,
	                            FString::Printf(TEXT("- `signalName: \"%s\"`"), *Signal->GetInternalName()));
	FINGenLuaSumnekoDescription(SignalDocumentation,
	                            FString::Printf(TEXT("- `component: %s`"), *Parent));

	for (const UFINProperty *Prop : Signal->GetParameters()) {
		FINGenLuaSumnekoDescription(SignalDocumentation,
		                            FString::Printf(
			                            TEXT("- `%s: %s` \n%s"),
			                            *Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")),
			                            *FINGenLuaSumnekoGetType(Ref, Prop),
			                            *Prop->GetDescription().ToString()
		                            ));
	}

	// hard coding the type is maybe not the best choice
	SignalDocumentation.Append(TEXT("---@deprecated\n---@type FIN.Signal\n"));
	SignalDocumentation.Appendf(
		TEXT("%s.%s = { isVarArgs = %s }\n"),
		*Parent,
		*Signal->GetInternalName(),
		Signal->IsVarArgs()
			? TEXT("true")
			: TEXT("false")
	);

	return SignalDocumentation;
}

FString FINGenLuaSumnekoClass(FFINReflection &Ref, const UFINClass *Class) {
	FString ClassTypeName = FINGenLuaSumnekoGetTypeName(Class);

	FString OperatorDocumentation;
	FString MembersDocumentation;

	for (const UFINProperty *Prop : Class->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			MembersDocumentation.Append(FINGenLuaSumnekoProperty(Ref, Class->GetInternalName(), Prop));
		}
	}

	for (const UFINFunction *Func : Class->GetFunctions(false)) {
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			if (Func->GetInternalName().Contains("FIN_Operator")) {
				auto SumnekoOperator = FINGenLuaSumnekoOperator(Ref, Func);
				if (SumnekoOperator) {
					OperatorDocumentation.Append(*SumnekoOperator);	
				}
				
				continue; // to next function
			}

			MembersDocumentation.Append(FINGenLuaSumnekoFunction(Ref, Class->GetInternalName(), Func));
		}
	}

	for (const UFINSignal *Signal : Class->GetSignals(false)) {
		MembersDocumentation.Append(FINGenLuaSumnekoSignal(Ref, Class->GetInternalName(), Signal));
	}

	FString ClassDocumentation = "\n";

	FINGenLuaSumnekoDescription(ClassDocumentation, Class->GetDescription().ToString());
	ClassDocumentation.Appendf(
		TEXT("---@class %s%s%s\n"),
		*ClassTypeName,
		*(Class->GetParent()
			  ? TEXT(" : ") + FINGenLuaSumnekoGetTypeName(Class->GetParent())
			  : TEXT("")),
		*OperatorDocumentation
	);

	FString ClassGlobalClassesDocumentation = FString::Printf(
		TEXT("---@class FIN.classes.%s : %s\nclasses.%s = nil\n"),
		*ClassTypeName,
		*ClassTypeName,
		*Class->GetInternalName()
	);

	return FString::Printf(TEXT("%slocal %s\n\n%s%s"),
	                       *ClassDocumentation, *Class->GetInternalName(), *ClassGlobalClassesDocumentation,
	                       *MembersDocumentation);
}

FString FINGenLuaSumnekoStruct(FFINReflection &Ref, const UFINStruct *Struct) {
	FString StructTypeName = FINGenLuaSumnekoGetTypeName(Struct);

	FString OperatorDocumentation;
	FString MembersDocumentation;
	TArray<FString> PropertyTypes;

	for (const UFINProperty *Prop : Struct->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			MembersDocumentation.Append(FINGenLuaSumnekoProperty(Ref, Struct->GetInternalName(), Prop));
			PropertyTypes.Add(FINGenLuaSumnekoGetType(Ref, Prop));
		}
	}

	for (const UFINFunction *Func : Struct->GetFunctions(false)) {
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			if (Func->GetInternalName().Contains("FIN_Operator")) {
				auto SumnekoOperator = FINGenLuaSumnekoOperator(Ref, Func);
				if (SumnekoOperator) {
					OperatorDocumentation.Append(*SumnekoOperator);	
				}
				continue; // to next function
			}

			MembersDocumentation.Append(FINGenLuaSumnekoFunction(Ref, Struct->GetInternalName(), Func));
		}
	}

	FString StructDocumentation = "\n";
	FINGenLuaSumnekoDescription(StructDocumentation, Struct->GetDescription().ToString());
	StructDocumentation.Appendf(
		TEXT("---@class %s%s\n"),
		*StructTypeName,
		*OperatorDocumentation
	);

	FString StructGlobalStructsDocumentation = FString::Printf(TEXT("---@class FIN.structs.%s : %s\n"), *StructTypeName, *StructTypeName);
	if (Struct->GetStructFlags() & FIN_Struct_Constructable) {
		FString ConstructorCallSignature = TEXT("{");
		for (int i = 0; i < PropertyTypes.Num(); ++i) {
			ConstructorCallSignature.Appendf(TEXT("%s [%d]: %s"),
			                                 i > 0
				                                 ? TEXT(",")
				                                 : TEXT(""),
			                                 i + 1, // offset because lua starts arrays at 1
			                                 *PropertyTypes[i]
			);
		}
		ConstructorCallSignature.Append(TEXT(" }"));

		StructGlobalStructsDocumentation.Appendf(
			TEXT("---@overload fun(data: %s ) : %s\n"),
			*ConstructorCallSignature,
			*StructTypeName
		);
	}
	StructGlobalStructsDocumentation.Appendf(TEXT("structs.%s = nil\n"), *Struct->GetInternalName());

	return FString::Printf(TEXT("%slocal %s\n\n%s%s"),
	                       *StructDocumentation, *Struct->GetInternalName(), *StructGlobalStructsDocumentation,
	                       *MembersDocumentation);
}

bool FINGenLuaDocSumneko(UWorld *World, const TCHAR *Command, FOutputDevice &Ar) {
	if (FParse::Command(&Command, TEXT("FINGenLuaDocSumneko"))) {
		FFINReflection &Ref = *FFINReflection::Get();
		FString Documentation = "\n";
		Documentation.Append(FINGenLuaSumnekoDocumentationStart);
		
		// adding "do" and "end" to get rid of local maximum variables reached
		int32_t count = 0;
		for (TPair<UClass*, UFINClass*> const Class : Ref.GetClasses()) {
			if (count == 0) {
				Documentation.Append("do\n");
			}

			Documentation.Append(FINGenLuaSumnekoClass(Ref, Class.Value));
			count++;

			if (count == 180) {
				Documentation.Append("\nend\n");
				count = 0;
			}
		}
		for (TPair<UScriptStruct*, UFINStruct*> const Struct : Ref.GetStructs()) {
			if (count == 0) {
				Documentation.Append("do\n");
			}

			Documentation.Append(FINGenLuaSumnekoStruct(Ref, Struct.Value));
			count++;

			if (count == 180) {
				Documentation.Append("\nend\n");
				count = 0;
			}
		}

		if (count != 0) {
			Documentation.Append("\nend\n");
		}

		Documentation.Append(FINGenLuaSumnekoDocumentationEnd);

		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINLuaDocumentationSumneko.lua"));
		FFileHelper::SaveStringToFile(Documentation, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

		return true;
	}
	return false;
}
#pragma optimize("", on)

[[maybe_unused]]
static FStaticSelfRegisteringExec FINGenLuaDocSumnekoStaticExec(&FINGenLuaDocSumneko);
