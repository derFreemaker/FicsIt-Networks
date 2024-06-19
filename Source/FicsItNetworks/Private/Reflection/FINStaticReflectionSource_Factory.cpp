#include "Reflection/FINStaticReflectionSourceHooks.h"
#include "Reflection/FINStaticReflectionSourceMacros.h"

#include "FGBuildableDoor.h"
#include "FGCategory.h"
#include "FGFactoryConnectionComponent.h"
#include "FGItemCategory.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Computer/FINComputerSubsystem.h"

BeginClass(UFGFactoryConnectionComponent, "FactoryConnection", "Factory Connection", "A actor component that is a connection point to which a conveyor or pipe can get attached to.")
Hook(UFINFactoryConnectorHook)
BeginSignal(ItemTransfer, "Item Transfer", "Triggers when the factory connection component transfers an item.")
	SignalParam(RStruct<FInventoryItem>, item, "Item", "The transfered item")
EndSignal()
BeginProp(RInt, type, "Type", "Returns the type of the connection. 0 = Conveyor, 1 = Pipe") {
	Return (int64)self->GetConnector();
} EndProp()
BeginProp(RInt, direction, "Direction", "The direction in which the items/fluids flow. 0 = Input, 1 = Output, 2 = Any, 3 = Used just as snap point") {
	Return (int64)self->GetDirection();
} EndProp()
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	Return self->IsConnected();
} EndProp()
BeginProp(RClass<UFGItemDescriptor>, allowedItem, "Allowed Item", "This item type defines which items are the only ones this connector can transfer. Null allows all items to be transfered.") {
	Return (FINClass)AFINComputerSubsystem::GetComputerSubsystem(self)->GetFactoryConnectorAllowedItem(self);
} PropSet() {
	AFINComputerSubsystem::GetComputerSubsystem(self)->SetFactoryConnectorAllowedItem(self, Val);
} EndProp()
BeginProp(RBool, blocked, "Blocked", "True if this connector doesn't transfer any items except the 'Unblocked Transfers'.") {
	Return AFINComputerSubsystem::GetComputerSubsystem(self)->GetFactoryConnectorBlocked(self);
} PropSet() {
	AFINComputerSubsystem::GetComputerSubsystem(self)->SetFactoryConnectorBlocked(self, Val);
} EndProp()
BeginProp(RInt, unblockedTransfers, "Unblocked Transfers", "The count of transfers that can still happen even if the connector is blocked. Use the 'AddUnblockedTransfers' function to change this. The count decreases by one when an item gets transfered.") {
	Return AFINComputerSubsystem::GetComputerSubsystem(self)->GetFactoryConnectorUnblockedTransfers(self);
} EndProp()
BeginFunc(addUnblockedTransfers, "Add Unblocked Transfers", "Adds the given count to the unblocked transfers counter. The resulting value gets clamped to >= 0. Negative values allow to decrease the counter manually. The returning int is the now set count.") {
	InVal(RInt, unblockedTransfers, "Unblocked Transfers", "The count of unblocked transfers to add.")
	OutVal(RInt, newUnblockedTransfers, "New Unblocked Transfers", "The new count of unblocked transfers.")
	FuncBody()
	newUnblockedTransfers = (FINInt) AFINComputerSubsystem::GetComputerSubsystem(self)->AddFactoryConnectorUnblockedTransfers(self, unblockedTransfers);
} EndFunc()
BeginFunc(getInventory, "Get Inventory", "Returns the internal inventory of the connection component.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The internal inventory of the connection component.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getConnected, "Get Connected", "Returns the connected factory connection component.") {
	OutVal(RTrace<UFGFactoryConnectionComponent>, connected, "Connected", "The connected factory connection component.")
	FuncBody()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(AFGBuildableManufacturer, "Manufacturer", "Manufacturer", "The base class of every machine that uses a recipe to produce something automatically.")
BeginFunc(getRecipe, "Get Recipe", "Returns the currently set recipe of the manufacturer.") {
	OutVal(RClass<UFGRecipe>, recipe, "Recipe", "The currently set recipe.")
	FuncBody()
	recipe = (UClass*)self->GetCurrentRecipe();
} EndFunc()
BeginFunc(getRecipes, "Get Recipes", "Returns the list of recipes this manufacturer can get set to and process.") {
	OutVal(RArray<RClass<UFGRecipe>>, recipes, "Recipes", "The list of avalible recipes.")
	FuncBody()
	TArray<FINAny> OutRecipes;
	TArray<TSubclassOf<UFGRecipe>> Recipes;
	self->GetAvailableRecipes(Recipes);
	for (TSubclassOf<UFGRecipe> Recipe : Recipes) {
		OutRecipes.Add((FINAny)(UClass*)Recipe);
	}
	recipes = OutRecipes;
} EndFunc()
BeginFunc(setRecipe, "Set Recipe", "Sets the currently producing recipe of this manufacturer.", 0) {
	InVal(RClass<UFGRecipe>, recipe, "Recipe", "The recipe this manufacturer should produce.")
	OutVal(RBool, gotSet, "Got Set", "True if the current recipe got successfully set to the new recipe.")
	FuncBody()
	TArray<TSubclassOf<UFGRecipe>> recipes;
	self->GetAvailableRecipes(recipes);
	if (recipes.Contains(recipe)) {
		TArray<FInventoryStack> stacks;
		self->GetInputInventory()->GetInventoryStacks(stacks);
		self->GetInputInventory()->Empty();
		self->GetOutputInventory()->AddStacks(stacks);
		self->SetRecipe(recipe);
		gotSet = self->GetCurrentRecipe() == recipe;
	} else {
		gotSet = false;
	}
} EndFunc()
BeginFunc(getInputInv, "Get Input Inventory", "Returns the input inventory of this manufacturer.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The input inventory of this manufacturer")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetInputInventory();
} EndFunc()
BeginFunc(getOutputInv, "Get Output Inventory", "Returns the output inventory of this manufacturer.") {
	OutVal(RTrace<UFGInventoryComponent>, inventory, "Inventory", "The output inventory of this manufacturer.")
	FuncBody()
	inventory = Ctx.GetTrace() / self->GetOutputInventory();
} EndFunc()
EndClass()

BeginClass(AFGBuildableDoor, "Door", "Door", "The base class of all doors.")
BeginFunc(getConfiguration, "Get Configuration", "Returns the Door Mode/Configuration.\n0 = Automatic\n1 = Always Closed\n2 = Always Open") {
	OutVal(RInt, configuration, "Configuration", "The current door mode/configuration.")
	FuncBody()
	configuration = FStaticReflectionSourceHelper::AFGBuildableDoor_GetConfig(self);
} EndFunc()
BeginFunc(setConfiguration, "Set Configuration", "Sets the Door Mode/Configuration, only some modes are allowed, if the mod you try to set is invalid, nothing changes.\n0 = Automatic\n1 = Always Closed\n2 = Always Open", 0) {
	InVal(RInt, configuration, "Configuration", "The new configuration for the door.")
	FuncBody()
	EDoorConfiguration Config = (EDoorConfiguration)FMath::Clamp(configuration, 0, 2);
	FStaticReflectionSourceHelper::AFGBuildableDoor_Update(self, Config);
} EndFunc()
EndClass()

BeginClass(UFGRecipe, "Recipe", "Recipe", "A struct that holds information about a recipe in its class. Means don't use it as object, use it as class type!")
BeginClassProp(RString, name, "Name", "The name of this recipe.") {
	Return (FINStr)UFGRecipe::GetRecipeName(self).ToString();
} EndProp()
BeginClassProp(RFloat, duration, "Duration", "The duration how much time it takes to cycle the recipe once.") {
	Return UFGRecipe::GetManufacturingDuration(self);
} EndProp()
BeginClassFunc(getProducts, "Get Products", "Returns a array of item amounts, this recipe returns (outputs) when the recipe is processed once.", false) {
	OutVal(RArray<RStruct<FItemAmount>>, products, "Products", "The products of this recipe.")
	FuncBody()
	TArray<FINAny> Products;
	for (const FItemAmount& Product : UFGRecipe::GetProducts(self)) {
		Products.Add((FINAny)Product);
	}
	products = Products;
} EndFunc()
BeginClassFunc(getIngredients, "Get Ingredients", "Returns a array of item amounts, this recipe needs (input) so the recipe can be processed.", false) {
	OutVal(RArray<RStruct<FItemAmount>>, ingredients, "Ingredients", "The ingredients of this recipe.")
	FuncBody()
	TArray<FINAny> Ingredients;
	for (const FItemAmount& Ingredient : UFGRecipe::GetIngredients(self)) {
		Ingredients.Add((FINAny)Ingredient);
	}
	ingredients = Ingredients;
} EndFunc()
EndClass()

BeginClass(UFGItemDescriptor, "ItemType", "Item Type", "The type of an item (iron plate, iron rod, leaves)")
BeginClassProp(RInt, form, "Form", "The matter state of this resource.\n1: Solid\n2: Liquid\n3: Gas\n4: Heat") {
	Return (FINInt)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, energy, "Energy", "How much energy this resource provides if used as fuel.") {
	Return (FINFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, radioactiveDecay, "Radioactive Decay", "The amount of radiation this item radiates.") {
	Return (FINFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RString, name, "Name", "The name of the item.") {
	Return (FINStr)UFGItemDescriptor::GetItemName(self).ToString();
} EndProp()
BeginClassProp(RString, description, "Description", "The description of this item.") {
	Return (FINStr)UFGItemDescriptor::GetItemDescription(self).ToString();
} EndProp()
BeginClassProp(RInt, max, "Max", "The maximum stack size of this item.") {
	Return (FINInt)UFGItemDescriptor::GetStackSize(self);
} EndProp()
BeginClassProp(RBool, canBeDiscarded, "Can be Discarded", "True if this item can be discarded.") {
	Return (FINBool)UFGItemDescriptor::CanBeDiscarded(self);
} EndProp()
BeginClassProp(RClass<UFGItemCategory>, category, "Category", "The category in which this item is in.") {
	Return (FINClass)UFGItemDescriptor::GetCategory(self);
} EndProp()
BeginClassProp(RStruct<FLinearColor>, fluidColor, "Fluid Color", "The color of this fluid.") {
	Return (FINStruct)(FLinearColor)UFGItemDescriptor::GetFluidColor(self);
} EndProp()
EndClass()

BeginClass(UFGItemCategory, "ItemCategory", "Item Category", "The category of some items.")
BeginClassProp(RString, name, "Name", "The name of the category.") {
	Return (FINStr)UFGItemCategory::GetCategoryName(self).ToString();
} EndProp()
EndClass()

BeginStructConstructable(FItemAmount, "ItemAmount", "Item Amount",
						 "A struct that holds a pair of amount and item type.")
	BeginProp(RInt, amount, "Amount", "The amount of items.") {
	Return (int64) self->Amount;
} PropSet() {
	self->Amount = Val;
} EndProp()
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the items.") {
	Return (UClass*)self->ItemClass;
} PropSet() {
	self->ItemClass = Val;
} EndProp()
EndStruct()

BeginStructConstructable(FInventoryStack, "ItemStack", "Item Stack", "A structure that holds item information and item amount to represent an item stack.")
BeginProp(RInt, count, "Count", "The count of items.") {
	Return (int64) self->NumItems;
} PropSet() {
	self->NumItems = Val;
} EndProp()
BeginProp(RStruct<FInventoryItem>, item, "Item", "The item information of this stack.") {
	Return self->Item;
} PropSet() {
	self->Item = Val;
} EndProp()
EndStruct()

BeginStructConstructable(FInventoryItem, "Item", "Item", "A structure that holds item information.")
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the item.") {
	Return (UClass*)self->GetItemClass();
} PropSet() {
	self->SetItemClass(Val);
} EndProp()
EndStruct()
