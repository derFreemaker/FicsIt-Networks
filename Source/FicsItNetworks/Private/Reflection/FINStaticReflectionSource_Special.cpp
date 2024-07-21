#include "FGGameState.h"
#include "FGIconLibrary.h"
#include "Reflection/FINStaticReflectionSourceMacros.h"

#include "FGResourceSinkSubsystem.h"
#include "FGSignLibrary.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableResourceSink.h"
#include "Buildables/FGBuildableSignBase.h"
#include "Buildables/FGBuildableWidgetSign.h"

BeginClass(AFGBuildableResourceSink, "ResourceSink", "Resource Sink", "The resource sink, also known a A.W.E.S.O.M.E Sink")
	BeginProp(RInt, numPoints, "Num Points", "The number of available points.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumTotalPoints(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RInt, numCoupons, "Num Coupons", "The number of available coupons to print.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumCoupons();
} EndProp()
BeginProp(RInt, numPointsToNextCoupon, "Num Points To Next Coupon", "The number of needed points for the next coupon.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumPointsToNextCoupon(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RFloat, couponProgress, "Coupon Progress", "The percentage of the progress for the next coupon.") {
	Return AFGResourceSinkSubsystem::Get(self)->GetProgressionTowardsNextCoupon(EResourceSinkTrack::RST_Default);
} EndProp()
EndClass()

BeginClass(AFGBuildableLightSource, "LightSource", "Light Source", "The base class for all light you can build.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the light is enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the light should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the light.") {
	return self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the light uses.") {
	return (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
} EndProp()
BeginFunc(getColorFromSlot, "Get Color from Slot", "Returns the light color that is referenced by the given slot.") {
	InVal(RInt, slot, "Slot", "The slot you want to get the referencing color from.")
	OutVal(RStruct<FLinearColor>, color, "Color", "The color this slot references.")
	FuncBody()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	color = (FINStruct) SubSys->GetBuildableLightColorSlot(slot);
} EndFunc()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	FuncBody()
	// AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightsControlPanel, "LightsControlPanel", "Light Source", "A control panel to configure multiple lights at once.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the lights should be enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightEnabled(Val);
	}
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the lights should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the lights.") {
	return self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the lights should use.") {
	return (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	FuncBody()
	// AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableSignBase, "SignBase", "Sign Base", "The base class for all signs in the game.")
BeginFunc(getSignType, "Get Sign Type", "Returns the sign type descriptor") {
	OutVal(RClass<UFGSignTypeDescriptor>, descriptor, "Descriptor", "The sign type descriptor")
	FuncBody()
	descriptor = (FINClass)IFGSignInterface::Execute_GetSignTypeDescriptor(self);
} EndFunc()
EndClass()

BeginClass(AFGBuildableWidgetSign, "WidgetSign", "Widget Sign", "The type of sign that allows you to define layouts, images, texts and colors manually.")
BeginFunc(setPrefabSignData, "Set Prefab Sign Data", "Sets the prefabg sign data e.g. the user settings like colo and more to define the signs content.", 0) {
	InVal(RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	FuncBody()
	self->SetPrefabSignData(prefabSignData);
} EndFunc()
BeginFunc(getPrefabSignData, "Get Prefab Sign Data", "Returns the prefabg sign data e.g. the user settings like colo and more to define the signs content.") {
	OutVal(RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	FuncBody()
	FPrefabSignData SignData;
	self->GetSignPrefabData(SignData);
	prefabSignData = (FINStruct)SignData;
} EndFunc()
EndClass()

BeginClass(UFGSignTypeDescriptor, "SignType", "Sign Type", "Describes the type of a sign.")
BeginClassProp(RStruct<FVector2D>, dimensions, "Dimensions", "The canvas dimensions of this sign.") {
	FVector2D dimensions;
	UFGSignLibrary::GetCanvasDimensionsFromSignDescriptor(self, dimensions);
	Return dimensions;
} EndProp()
BeginClassFunc(getColors, "Get Colors", "Returns the default foreground/background/auxiliary colors of this sign type.", false) {
	OutVal(RStruct<FLinearColor>, foreground, "Foreground", "The foreground color")
	OutVal(RStruct<FLinearColor>, background, "Background", "The background color")
	OutVal(RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary color")
	FuncBody()
	FLinearColor fg, bg, au;
	UFGSignLibrary::GetDefaultColorsFromSignDescriptor(self, fg, bg, au);
	foreground = (FINStruct)fg;
	background = (FINStruct)bg;
	auxiliary = (FINStruct)au;
} EndFunc()
BeginClassFunc(getPrefabs, "Get Prefabs", "Returns a list of all sign prefabs this sign can use.", false) {
	OutVal(RArray<RClass<UFGSignPrefabWidget>>, prefabs, "Prefabs", "The sign prefabs this sign can use")
	FuncBody()
	TArray<FINAny> PrefabsArray;
	TArray<TSubclassOf<UFGSignPrefabWidget>> PrefabList;
	UFGSignLibrary::GetPrefabLayoutsFromSignDescriptor(self, PrefabList);
	for (TSubclassOf<UFGSignPrefabWidget> Prefab : PrefabList) {
		PrefabsArray.Add((FINClass)Prefab);
	}
	prefabs = PrefabsArray;
} EndFunc()
BeginClassFunc(getTextElements, "Get Text Elements", "Returns a list of element names and their default text values.", false) {
	OutVal(RArray<RString>, textElements, "Text Elements", "A list of text element names of this type.")
	OutVal(RArray<RString>, textElementsDefaultValues, "Text Elements Default Values", "A list of default values for the text elements of this type.")
	FuncBody()
	TArray<FINAny> TextElements, TextElementsDefaultValues;
	TMap<FString, FString> Elements;
	UFGSignLibrary::GetTextElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, FString>& Element : Elements) {
		TextElements.Add(Element.Key);
		TextElementsDefaultValues.Add(Element.Value);
	}
	textElements = TextElements;
	textElementsDefaultValues = TextElementsDefaultValues;
} EndFunc()
BeginClassFunc(getIconElements, "Get Icon Elements", "Returns a list of element names and their default icon values.", false) {
	OutVal(RArray<RString>, iconElements, "Icon Elements", "A list of icon element names of this type.")
	OutVal(RArray<RObject<UTexture2D>>, iconElementsDefaultValues, "Icon Elements Default Values", "A list of default values for the icon elements of this type.")
	FuncBody()
	TArray<FINAny> IconElements, IconElementsDefaultValues;
	TMap<FString, UObject*> Elements;
	UFGSignLibrary::GetIconElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, UObject*>& Element : Elements) {
		IconElements.Add(Element.Key);
		IconElementsDefaultValues.Add((FINObj)Element.Value);
	}
	iconElements = IconElements;
	iconElementsDefaultValues = IconElementsDefaultValues;
} EndFunc()
EndClass()

BeginClass(UFGSignPrefabWidget, "SignPrefab", "Sign Prefab", "Descibes a layout of a sign.")
EndClass()

BeginStructConstructable(FPrefabSignData, "PrefabSignData", "Prefab Sign Data", "This structure stores all data that defines what a sign displays.")
BeginProp(RClass<UObject>, layout, "Layout", "The object that actually displayes the layout") {
	Return (FINClass)self->PrefabLayout;
} PropSet() {
	self->PrefabLayout = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, foreground, "Foreground", "The foreground Color.") {
	Return (FINStruct)self->ForegroundColor;
} PropSet() {
	self->ForegroundColor = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, background, "bBckground", "The background Color.") {
	Return (FINStruct)self->BackgroundColor;
} PropSet() {
	self->BackgroundColor = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary Color.") {
	Return (FINStruct)self->AuxiliaryColor;
} PropSet() {
	self->AuxiliaryColor = Val;
} EndProp()
BeginProp(RClass<UFGSignTypeDescriptor>, signType, "Sign Type", "The type of sign this prefab fits to.") {
	Return (FINClass)self->SignTypeDesc;
} PropSet() {
	self->SignTypeDesc = Val;
} EndProp()
BeginFunc(getTextElements, "Get Text Elements", "Returns all text elements and their values.") {
	OutVal(RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	OutVal(RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	FuncBody()
	TArray<FINAny> TextElements, TextElementValues;
	for (const TPair<FString, FString>& Element : self->TextElementData) {
		TextElements.Add(Element.Key);
		TextElementValues.Add(Element.Value);
	}
	textElements = (FINArray)TextElements;
	textElementValues = (FINArray)TextElementValues;
} EndFunc()
BeginFunc(getIconElements, "Get Icon Elements", "Returns all icon elements and their values.") {
	OutVal(RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	OutVal(RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	FuncBody()
	TArray<FINAny> IconElements, IconElementValues;
	for (const TPair<FString, int32>& Element : self->IconElementData) {
		IconElements.Add(Element.Key);
		IconElementValues.Add((FINInt)Element.Value);
	}
	iconElements = IconElements;
	iconElementValues = IconElementValues;
} EndFunc()
BeginFunc(setTextElements, "Set Text Elements", "Sets all text elements and their values.") {
	InVal(RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	InVal(RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	FuncBody()
	if (textElements.Num() != textElementValues.Num()) throw FFINException(TEXT("Count of element names and element values are not the same."));
	self->TextElementData.Empty();
	for (int i = 0; i < textElements.Num(); ++i) {
		self->TextElementData.Add(textElements[i].GetString(), textElementValues[i].GetString());
	}
} EndFunc()
BeginFunc(setIconElements, "Set Icon Elements", "Sets all icon elements and their values.") {
	InVal(RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	InVal(RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	FuncBody()
	if (iconElements.Num() != iconElementValues.Num()) throw FFINException(TEXT("Count of element names and element values are not the same."));
	self->IconElementData.Empty();
	for (int i = 0; i < iconElements.Num(); ++i) {
		self->IconElementData.Add(iconElements[i].GetString(), iconElementValues[i].GetInt());
	}
} EndFunc()
BeginFunc(setTextElement, "Set Text Element", "Sets a text element with the given element name.") {
	InVal(RString, elementName, "Element Name", "The name of the text element")
	InVal(RString, value, "Value", "The value of the text element")
	FuncBody()
	self->TextElementData.Add(elementName, value);
} EndFunc()
BeginFunc(setIconElement, "Set Icon Element", "Sets a icon element with the given element name.") {
	InVal(RString, elementName, "Element Name", "The name of the icon element")
	InVal(RInt, value, "Value", "The value of the icon element")
	FuncBody()
	self->IconElementData.Add(elementName, value);
} EndFunc()
BeginFunc(getTextElement, "Get Text Element", "Gets a text element with the given element name.") {
	InVal(RString, elementName, "Element Name", "The name of the text element")
	OutVal(RInt, value, "Value", "The value of the text element")
	FuncBody()
	FString* Element = self->TextElementData.Find(elementName);
	if (!Element) throw FFINException(TEXT("No element with the given name found"));
	value = *Element;
} EndFunc()
BeginFunc(getIconElement, "Get Icon Element", "Gets a icon element with the given element name.") {
	InVal(RString, elementName, "Element Name", "The name of the icon element")
	OutVal(RInt, value, "Value", "The value of the icon element")
	FuncBody()
	int* Element = self->IconElementData.Find(elementName);
	if (!Element) throw FFINException(TEXT("No element with the given name found"));
	value = (FINInt)*Element;
} EndFunc()
EndStruct()

BeginStructConstructable(FLinearColor, "Color", "Color", "A structure that holds a rgba color value")
BeginProp(RFloat, r, "Red", "The red portion of the color.") {
	Return (FINFloat) self->R;
} PropSet() {
	self->R = Val;
} EndProp()
BeginProp(RFloat, g, "Green", "The green portion of the color.") {
	Return (FINFloat) self->G;
} PropSet() {
	self->G = Val;
} EndProp()
BeginProp(RFloat, b, "Blue", "The blue portion of the color.") {
	Return (FINFloat) self->B;
} PropSet() {
	self->B = Val;
} EndProp()
BeginProp(RFloat, a, "Alpha", "The alpha (opacity) portion of the color.") {
	Return (FINFloat) self->A;
} PropSet() {
	self->A = Val;
} EndProp()
BeginOp(FIN_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.") {
	InVal(RStruct<FLinearColor>, other, "Other", "The other color that should be added to this color")
	OutVal(RStruct<FLinearColor>, result, "Result", "The resulting color of the color addition")
	FuncBody()
	result = (FINStruct)(*self + other);
} EndFunc()
BeginOp(FIN_Operator_Neg, 1, "Operator Neg", "The Negation operator for this struct. Does NOT make the color negative. Calculates 1 - this.") {
	OutVal(RStruct<FLinearColor>, result, "Result", "The resulting color of the color addition")
	FuncBody()
	result = (FINStruct)(FLinearColor::White - *self);
} EndFunc()
BeginOp(FIN_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.") {
	InVal(RStruct<FLinearColor>, other, "Other", "The other color that should be subtracted from this color")
	OutVal(RStruct<FLinearColor>, result, "Result", "The resulting color of the color subtraction")
	FuncBody()
	result = (FINStruct)(*self - other);
} EndFunc()
BeginOp(FIN_Operator_Mul, 1, "Color Factor Scaling", "") {
	InVal(RFloat, factor, "Factor", "The factor with which this color should be scaled with.")
	OutVal(RStruct<FVector>, result, "Result", "The resulting scaled color.")
	FuncBody()
	result = (FINStruct)(*self * factor);
} EndFunc()
BeginOp(FIN_Operator_Div, 1, "Color Inverse Factor Scaling", "") {
	InVal(RFloat, factor, "Factor", "The factor with which this color should be scaled inversly with.")
	OutVal(RStruct<FVector>, result, "Result", "The resulting inverse scaled color.")
	FuncBody()
	result = (FINStruct)(*self / factor);
} EndFunc()
EndStruct()

BeginStruct(FIconData, "IconData", "Icon Data", "A struct containing information about a game icon (used in f.e. signs).")
BeginProp(RBool, isValid, "Is Valid", "True if the icon data refers to an valid icon") {
	Return FINBool(self->ID >= 0);
} EndProp()
BeginProp(RInt, id, "ID", "The icon ID.") {
	Return (FINInt)self->ID;
} EndProp()
BeginProp(RString, ref, "Ref", "The media reference of this icon.") {
	Return FString::Printf(TEXT("icon:%i"), self->ID);
} EndProp()
BeginProp(RBool, animated, "Animated", "True if the icon is animated.") {
	Return self->Animated;
} EndProp()
BeginProp(RString, iconName, "Icon Name", "The name of the icon.") {
	Return self->IconName.ToString();
} EndProp()
BeginProp(RString, iconType, "Icon Type", "The type of the icon.\n0 = Building\n1 = Part\n2 = Equipment\n3 = Monochrome\n4 = Material\n5 = Custom\n6 = Map Stamp") {
	Return (FINInt)self->IconType;
} EndProp()
BeginProp(RBool, hidden, "Hidden", "True if the icon is hidden in the selection.") {
	Return self->Hidden;
} EndProp()
BeginProp(RBool, searchOnly, "Search Only", "True if the icon will be shown in selection only if searched for directly by name.") {
	Return self->SearchOnly;
} EndProp()
EndStruct()

BeginStructConstructable(FMargin, "Margin", "Margin", "A struct containing four floats that describe a margin around a box (like a 9-patch).")
BeginProp(RFloat, left, "Left", "The left edge of the rectangle.") {
	Return FINFloat(self->Left);
} PropSet() {
	self->Left = Val;
} EndProp()
BeginProp(RFloat, right, "Right", "The right edge of the rectangle.") {
	Return FINFloat(self->Right);
} PropSet() {
	self->Right = Val;
} EndProp()
BeginProp(RFloat, top, "Top", "The top edge of the rectangle.") {
	Return FINFloat(self->Top);
} PropSet() {
	self->Top = Val;
} EndProp()
BeginProp(RFloat, bottom, "Bottom", "The bottom edge of the rectangle.") {
	Return FINFloat(self->Left);
} PropSet() {
	self->Bottom = Val;
} EndProp()
EndStruct()
