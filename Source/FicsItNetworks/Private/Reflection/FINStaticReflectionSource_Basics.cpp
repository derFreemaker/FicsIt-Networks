#include "Reflection/FINStaticReflectionSourceHooks.h"
#include "Reflection/FINStaticReflectionSourceMacros.h"

#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGPowerConnectionComponent.h"
#include "Network/FINNetworkConnectionComponent.h"

#include "UObject/Object.h"

BeginClass(UObject, "Object", "Object", "The base class of every object.")
BeginProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	Return (int64)GetTypeHash(self);
} EndProp()
BeginProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	Return (FINStr) self->GetName();
} EndProp()
BeginProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	Return (FINStr) self->GetPathName();
} EndProp()
BeginFunc(getHash, "Get Hash", "Returns a hash of this object. This is a value that nearly uniquely identifies this object.") {
	OutVal(RInt, hash, "Hash", "The hash of this object.");
	FuncBody()
	hash = (int64)GetTypeHash(self);
} EndFunc()
BeginFunc(getType, "Get Type", "Returns the type (aka class) of this object.") {
	OutVal(RObject<UFINClass>, type, "Type", "The type of this object");
	FuncBody()
	if (self) type = (FINObj)FFINReflection::Get()->FindClass(self->GetClass());
} EndFunc()
BeginFunc(isA, "Is A", "Checks if this Object is a child of the given typen.") {
	InVal(RClass<UObject>, parent, "Parent", "The parent we check if this object is a child of.")
	OutVal(RBool, isChild, "Is Child", "True if this object is a child of the given type.")
	FuncBody()
	isChild = (FINBool)self->IsA(parent);
} EndFunc()
BeginClassProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	Return (int64)GetTypeHash(self);
} EndProp()
BeginClassProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	Return (FINStr) self->GetName();
} EndProp()
BeginClassProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	Return (FINStr) self->GetPathName();
} EndProp()
BeginClassFunc(getHash, "Get Hash", "Returns the hash of this class. This is a value that nearly uniquely idenfies this object.", false) {
	OutVal(RInt, hash, "Hash", "The hash of this class.");
	FuncBody()
	hash = (int64) GetTypeHash(self);
} EndFunc()
BeginClassFunc(getType, "Get Type", "Returns the type (aka class) of this class instance.", false) {
	OutVal(RObject<UFINClass>, type, "Type", "The type of this class instance");
	FuncBody()
    if (self) type = (FINObj)FFINReflection::Get()->FindClass(self);
} EndFunc()
BeginClassFunc(isChildOf, "Is Child Of", "Checks if this Type is a child of the given typen.", false) {
	InVal(RClass<UObject>, parent, "Parent", "The parent we check if this type is a child of.")
	OutVal(RBool, isChild, "Is Child", "True if this type is a child of the given type.")
	FuncBody()
	isChild = (FINBool)self->IsChildOf(parent);
} EndFunc()
EndClass()

BeginClass(UFINBase, "ReflectionBase", "Reflection Base", "The base class for all things of the reflection system.")
BeginProp(RString, name, "Name", "The internal name.") {
	Return self->GetInternalName();
} EndProp()
BeginProp(RString, displayName, "Display Name", "The display name used in UI which might be localized.") {
	Return self->GetDisplayName().ToString();
} EndProp()
BeginProp(RString, description, "Description", "The description of this base.") {
	Return self->GetDescription().ToString();
} EndProp()
EndClass()

BeginClass(UFINStruct, "Struct", "Struct", "Reflection Object that holds information about structures.")
BeginProp(RBool, isConstructable, "Is Constructable", "True if this struct can be constructed by the user directly.") {
	Return (FINBool)(self->GetStructFlags() & FIN_Struct_Constructable);
} EndProp()
BeginFunc(getParent, "Get Parent", "Returns the parent type of this type.", false) {
	OutVal(RObject<UFINClass>, parent, "Parent", "The parent type of this type.");
	FuncBody()
    if (self) parent = (FINObj)self->GetParent();
} EndFunc()
BeginFunc(getProperties, "Get Properties", "Returns all the properties of this type.") {
	OutVal(RArray<RObject<UFINProperty>>, properties, "Properties", "The properties this specific type implements (excluding properties from parent types).")
	FuncBody()
	TArray<FINAny> Props;
	for (UFINProperty* Prop : self->GetProperties(false)) Props.Add((FINObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getAllProperties, "Get All Properties", "Returns all the properties of this and parent types.") {
	OutVal(RArray<RObject<UFINProperty>>, properties, "Properties", "The properties this type implements including properties from parent types.")
    FuncBody()
    TArray<FINAny> Props;
	for (UFINProperty* Prop : self->GetProperties(true)) Props.Add((FINObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getFunctions, "Get Functions", "Returns all the functions of this type.") {
	OutVal(RArray<RObject<UFINFunction>>, functions, "Functions", "The functions this specific type implements (excluding properties from parent types).")
    FuncBody()
    TArray<FINAny> Funcs;
	for (UFINFunction* Func : self->GetFunctions(false)) Funcs.Add((FINObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(getAllFunctions, "Get All Functions", "Returns all the functions of this and parent types.") {
	OutVal(RArray<RObject<UFINProperty>>, functions, "Functions", "The functions this type implements including functions from parent types.")
    FuncBody()
    TArray<FINAny> Funcs;
	for (UFINFunction* Func : self->GetFunctions(true)) Funcs.Add((FINObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(isChildOf, "Is Child Of", "Allows to check if this struct is a child struct of the given struct or the given struct it self.") {
	InVal(RObject<UFINStruct>, parent, "Parent", "The parent struct you want to check if this struct is a child of.")
    OutVal(RBool, isChild, "Is Child", "True if this struct is a child of parent.")
    FuncBody()
    if (self && parent.IsValid()) isChild = self->IsChildOf(Cast<UFINStruct>(parent.Get()));
} EndFunc()
EndClass()

BeginClass(UFINClass, "Class", "Class", "Object that contains all information about a type.")
BeginFunc(getSignals, "Get Signals", "Returns all the signals of this type.") {
	OutVal(RArray<RObject<UFINSignal>>, signals, "Signals", "The signals this specific type implements (excluding properties from parent types).")
    FuncBody()
    TArray<FINAny> Sigs;
	for (UFINSignal* Sig : self->GetSignals(false)) Sigs.Add((FINObj)Sig);
	signals = Sigs;
} EndFunc()
BeginFunc(getAllSignals, "Get All Signals", "Returns all the signals of this and its parent types.") {
	OutVal(RArray<RObject<UFINSignal>>, signals, "Signals", "The signals this type and all it parents implement.")
    FuncBody()
    TArray<FINAny> Sigs;
	for (UFINSignal* Sig : self->GetSignals(true)) Sigs.Add((FINObj)Sig);
	signals = Sigs;
} EndFunc()
EndClass()

BeginClass(UFINProperty, "Property", "Property", "A Reflection object that holds information about properties and parameters.")
BeginProp(RInt, dataType, "Data Type", "The data type of this property.\n0: nil, 1: bool, 2: int, 3: float, 4: str, 5: object, 6: class, 7: trace, 8: struct, 9: array, 10: anything") {
	Return (FINInt)self->GetType().GetValue();
} EndProp()
BeginProp(RInt, flags, "Flags", "The property bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this property a member attribute.\nIs this property read only.\nIs this property a parameter.\nIs this property a output paramter.\nIs this property a return value.\nCan this property get accessed in syncrounus runtime.\nCan this property can get accessed in parallel runtime.\nCan this property get accessed in asynchronus runtime.\nThis property is a class attribute.") {
	Return (FINInt) self->GetPropertyFlags();
} EndProp()
EndClass()

BeginClass(UFINArrayProperty, "ArrayProperty", "Array Property", "A reflection object representing a array property.")
BeginFunc(getInner, "Get Inner", "Returns the inner type of this array.") {
	OutVal(RObject<UFINProperty>, inner, "Inner", "The inner type of this array.")
	FuncBody()
	inner = (FINObj) self->GetInnerType();
} EndFunc()
EndClass()

BeginClass(UFINObjectProperty, "ObjectProperty", "Object Property", "A reflection object representing a object property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this object. Meaning, the stored objects need to be of this type.") {
	OutVal(RObject<UFINClass>, subclass, "Subclass", "The subclass of this object.")
    FuncBody()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINTraceProperty, "TraceProperty", "Trace Property", "A reflection object representing a trace property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this trace. Meaning, the stored traces need to be of this type.") {
	OutVal(RObject<UFINClass>, subclass, "Subclass", "The subclass of this trace.")
    FuncBody()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINClassProperty, "ClassProperty", "Class Property", "A reflection object representing a class property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this class. Meaning, the stored classes need to be of this type.") {
	OutVal(RObject<UFINClass>, subclass, "Subclass", "The subclass of this class property.")
    FuncBody()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINStructProperty, "StructProperty", "Struct Property", "A reflection object representing a struct property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this struct. Meaning, the stored structs need to be of this type.") {
	OutVal(RObject<UFINStruct>, subclass, "Subclass", "The subclass of this struct.")
    FuncBody()
    subclass = (FINObj) FFINReflection::Get()->FindStruct(self->GetInner());
} EndFunc()
EndClass()

BeginClass(UFINFunction, "Function", "Function", "A reflection object representing a function.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this function.") {
	OutVal(RArray<RObject<UFINProperty>>, parameters, "Parameters", "The parameters this function.")
    FuncBody()
    TArray<FINAny> ParamArray;
	for (UFINProperty* Param : self->GetParameters()) ParamArray.Add((FINObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RInt, flags, "Flags", "The function bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this function has a variable amount of input parameters.\nCan this function get called in syncrounus runtime.\nCan this function can get called in parallel runtime.\nCan this function get called in asynchronus runtime.\nIs this function a member function.\nThe function is a class function.\nThe function is a static function.\nThe function has a variable amount of return values.") {
	Return (FINInt) self->GetFunctionFlags();
} EndProp()
EndClass()

BeginClass(UFINSignal, "Signal", "Signal", "A reflection object representing a signal.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this signal.") {
	OutVal(RArray<RObject<UFINProperty>>, parameters, "Parameters", "The parameters this signal.")
    FuncBody()
    TArray<FINAny> ParamArray;
	for (UFINProperty* Param : self->GetParameters()) ParamArray.Add((FINObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RBool, isVarArgs, "Is VarArgs", "True if this signal has a variable amount of arguments.") {
	Return (FINBool) self->IsVarArgs();
} EndProp()
EndClass()

BeginClass(AActor, "Actor", "Actor", "This is the base class of all things that can exist within the world by them self.")
BeginProp(RStruct<FVector>, location, "Location", "The location of the actor in the world.") {
	Return self->GetActorLocation();
} EndProp()
BeginProp(RStruct<FVector>, scale, "Scale", "The scale of the actor in the world.") {
	Return self->GetActorScale();
} EndProp()
BeginProp(RStruct<FRotator>, rotation, "Rotation", "The rotation of the actor in the world.") {
	Return self->GetActorRotation();
} EndProp()
BeginFunc(getPowerConnectors, "Get Power Connectors", "Returns a list of power connectors this actor might have.") {
	OutVal(RArray<RTrace<UFGPowerConnectionComponent>>, connectors, "Connectors", "The power connectors this actor has.");
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPowerConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getFactoryConnectors, "Get Factory Connectors", "Returns a list of factory connectors this actor might have.") {
	OutVal(RArray<RTrace<UFGFactoryConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.");
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGFactoryConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getPipeConnectors, "Get Pipe Connectors", "Returns a list of pipe (fluid & hyper) connectors this actor might have.") {
	OutVal(RArray<RTrace<UFGPipeConnectionComponentBase>>, connectors, "Connectors", "The pipe connectors this actor has.");
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	TSet<UObject*> Outputted;
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPipeConnectionComponentBase::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		if (Outputted.Contains(Connector)) continue;
		Outputted.Add(Connector);
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getInventories, "Get Inventories", "Returns a list of inventories this actor might have.") {
	OutVal(RArray<RTrace<UFGInventoryComponent>>, inventories, "Inventories", "The inventories this actor has.");
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGInventoryComponent::StaticClass())) continue;
		UObject* inventory = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(inventory))) continue;
		Output.Add(Ctx.GetTrace() / inventory);
	}
	inventories = Output;
} EndFunc()
BeginFunc(getNetworkConnectors, "Get Network Connectors", "Returns the name of network connectors this actor might have.") {
	OutVal(RArray<RTrace<UFINNetworkConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.")
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFINNetworkConnectionComponent::StaticClass())) continue;
		UObject* connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(connector))) continue;
		Output.Add(Ctx.GetTrace() / connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getComponents, "Get Components", "Returns the components that make-up this actor.") {
	InVal(RClass<UActorComponent>, componentType, "Component Type", "The class will be used as filter.")
	OutVal(RArray<RTrace<UActorComponent>>, components, "Components", "The components of this actor.")
	FuncBody()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(componentType)) continue;
		UObject* component = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(component))) continue;
		Output.Add(Ctx.GetTrace() / component);
	}
	components = Output;
} EndFunc()
EndClass()

BeginClass(UActorComponent, "ActorComponent", "Actor Component", "A component/part of an actor in the world.")
BeginProp(RTrace<AActor>, owner, "Owner", "The parent actor of which this component is part of") {
	return Ctx.GetTrace() / self->GetOwner();
} EndProp()
EndClass()

BeginClass(AFGBuildable, "Buildable", "Buildable", "The base class of all buildables.")
Hook(UFINBuildableHook)
BeginSignal(ProductionChanged, "Production Changed", "Triggers when the production state of the buildable changes.")
	SignalParam(RInt, state, "State", "The new production state.")
EndSignal()
BeginProp(RInt, numPowerConnections, "Num Power Connection", "The count of available power connections this building has.") {
	Return (FINInt)self->GetNumPowerConnections();
} EndProp()
BeginProp(RInt, numFactoryConnections, "Num Factory Connection", "The cound of available factory connections this building has.") {
	Return (FINInt)self->GetNumFactoryConnections();
} EndProp()
BeginProp(RInt, numFactoryOutputConnections, "Num Factory Output Connection", "The count of available factory output connections this building has.") {
	Return (FINInt)self->GetNumFactoryOuputConnections();
} EndProp()
EndClass()

BeginClass(AFGBuildableFactory, "Factory", "Factory", "The base class of most machines you can build.")
BeginProp(RFloat, progress, "Progress", "The current production progress of the current production cycle.") {
	Return self->GetProductionProgress();
} EndProp()
BeginProp(RFloat, powerConsumProducing,	"Producing Power Consumption", "The power consumption when producing.") {
	Return self->GetProducingPowerConsumption();
} EndProp()
BeginProp(RFloat, productivity,	"Productivity", "The productivity of this factory.") {
	Return self->GetProductivity();
} EndProp()
BeginProp(RFloat, cycleTime, "Cycle Time", "The time that passes till one production cycle is finsihed.") {
	Return self->GetProductionCycleTime();
} EndProp()
BeginProp(RFloat, maxPotential, "Max Potential", "The maximum potential this factory can be set to.") {
	Return self->GetMaxPossiblePotential();
} EndProp()
BeginProp(RFloat, minPotential, "Min Potential", "The minimum potential this factory needs to be set to.") {
	Return self->GetMinPotential();
} EndProp()
BeginProp(RBool, standby, "Standby", "True if the factory is in standby.") {
	Return self->IsProductionPaused();
} PropSet() {
	self->SetIsProductionPaused(Val);
} EndProp()
BeginProp(RFloat, potential, "Potential", "The potential this factory is currently set to. (the overclock value)\n 0 = 0%, 1 = 100%") {
	Return self->GetPendingPotential();
} PropSet() {
	float min = self->GetMinPotential();
	float max = self->GetMaxPossiblePotential();
	self->SetPendingPotential(FMath::Clamp((float)Val, min, max));
} EndProp()
EndClass()

BeginStructConstructable(FVector2D, "Vector2D", "Vector 2D", "Contains two cordinates (X, Y) to describe a position or movement vector in 2D Space")
BeginProp(RFloat, x, "X", "The X coordinate component") {
	Return self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component") {
	Return self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginOp(FIN_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.") {
	InVal(RStruct<FVector2D>, other, "Other", "The other vector that should be added to this vector")
	OutVal(RStruct<FVector2D>, result, "Result", "The resulting vector of the vector addition")
	FuncBody()
	result = (FINStruct)(*self + other);
} EndFunc()
BeginOp(FIN_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.") {
	InVal(RStruct<FVector2D>, other, "Other", "The other vector that should be subtracted from this vector")
	OutVal(RStruct<FVector2D>, result, "Result", "The resulting vector of the vector subtraction")
	FuncBody()
	result = (FINStruct)(*self - other);
} EndFunc()
BeginOp(FIN_Operator_Neg, 0, "Operator Neg", "The Negation operator for this struct.") {
	OutVal(RStruct<FVector2D>, result, "Result", "The resulting vector of the vector negation")
	FuncBody()
	result = (FINStruct)(-*self);
} EndFunc()
BeginOp(FIN_Operator_Mul, 0, "Scalar Product", "") {
	InVal(RStruct<FVector2D>, other, "Other", "The other vector to calculate the scalar product with.")
	OutVal(RFloat, result, "Result", "The resulting scalar product.")
	FuncBody()
	result = (FINStruct)(*self * other);
} EndFunc()
BeginOp(FIN_Operator_Mul, 1, "Vector Factor Scaling", "") {
	InVal(RFloat, factor, "Factor", "The factor with which this vector should be scaled with.")
	OutVal(RStruct<FVector2D>, result, "Result", "The resulting scaled vector.")
	FuncBody()
	result = (FINStruct)(*self * factor);
} EndFunc()
EndStruct()

BeginStructConstructable(FVector, "Vector", "Vector", "Contains three cordinates (X, Y, Z) to describe a position or movement vector in 3D Space")
BeginProp(RFloat, x, "X", "The X coordinate component") {
	Return self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component") {
	Return self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginProp(RFloat, z, "Z", "The Z coordinate component") {
	Return self->Z;
} PropSet() {
	self->Z = Val;
} EndProp()
BeginOp(FIN_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.") {
	InVal(RStruct<FVector>, other, "Other", "The other vector that should be added to this vector")
	OutVal(RStruct<FVector>, result, "Result", "The resulting vector of the vector addition")
	FuncBody()
	result = (FINStruct)(*self + other);
} EndFunc()
BeginOp(FIN_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.") {
	InVal(RStruct<FVector>, other, "Other", "The other vector that should be subtracted from this vector")
	OutVal(RStruct<FVector>, result, "Result", "The resulting vector of the vector subtraction")
	FuncBody()
	result = (FINStruct)(*self - other);
} EndFunc()
BeginOp(FIN_Operator_Neg, 0, "Operator Neg", "The Negation operator for this struct.") {
	OutVal(RStruct<FVector>, result, "Result", "The resulting vector of the vector negation")
	FuncBody()
	result = (FINStruct)(-*self);
} EndFunc()
BeginOp(FIN_Operator_Mul, 0, "Scalar Product", "") {
	InVal(RStruct<FVector>, other, "Other", "The other vector to calculate the scalar product with.")
	OutVal(RFloat, result, "Result", "The resulting scalar product.")
	FuncBody()
	result = (FINStruct)(*self * other);
} EndFunc()
BeginOp(FIN_Operator_Mul, 1, "Vector Factor Scaling", "") {
	InVal(RFloat, factor, "Factor", "The factor with which this vector should be scaled with.")
	OutVal(RStruct<FVector>, result, "Result", "The resulting scaled vector.")
	FuncBody()
	result = (FINStruct)(*self * factor);
} EndFunc()
EndStruct()

BeginStructConstructable(FRotator, "Rotator", "Rotator", "Contains rotation information about a object in 3D spaces using 3 rotation axis in a gimble.")
BeginProp(RFloat, pitch, "Pitch", "The pitch component") {
	Return self->Pitch;
} PropSet() {
	self->Pitch = Val;
} EndProp()
BeginProp(RFloat, yaw, "Yaw", "The yaw component") {
	Return self->Yaw;
} PropSet() {
	self->Yaw = Val;
} EndProp()
BeginProp(RFloat, roll, "Roll", "The roll component") {
	Return self->Roll;
} PropSet() {
	self->Roll = Val;
} EndProp()
BeginOp(FIN_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.") {
	InVal(RStruct<FRotator>, other, "Other", "The other rotator that should be added to this rotator")
	OutVal(RStruct<FRotator>, result, "Result", "The resulting rotator of the vector addition")
	FuncBody()
	result = (FINStruct)(*self + other);
} EndFunc()
BeginOp(FIN_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.") {
	InVal(RStruct<FRotator>, other, "Other", "The other rotator that should be subtracted from this rotator")
	OutVal(RStruct<FRotator>, result, "Result", "The resulting rotator of the vector subtraction")
	FuncBody()
	result = (FINStruct)(*self - other);
} EndFunc()
EndStruct()

BeginStructConstructable(FVector4, "Vector4", "Vector4", "A Vector containing four values.")
BeginProp(RFloat, x, "X", "The first value in the Vector4.") {
	Return self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The second value in the Vector4.") {
	Return self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginProp(RFloat, z, "Z", "The third value in the Vector4.") {
	Return self->Z;
} PropSet() {
	self->Z = Val;
} EndProp()
BeginProp(RFloat, w, "W", "The fourth value in the Vector4.") {
	Return self->W;
} PropSet() {
	self->W = Val;
} EndProp()
EndStruct()
