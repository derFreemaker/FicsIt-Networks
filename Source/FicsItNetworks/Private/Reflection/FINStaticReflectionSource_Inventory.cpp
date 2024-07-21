#include "Reflection/FINStaticReflectionSourceMacros.h"

#include "FGInventoryComponent.h"
#include "FGInventoryLibrary.h"
#include "FicsItKernel/Logging.h"
#include "Replication/FGReplicationDetailActor.h"
#include "Replication/FGReplicationDetailActorOwnerInterface.h"

BeginClass(UFGInventoryComponent, "Inventory", "Inventory", "A actor component that can hold multiple item stacks.\nWARNING! Be aware of container inventories, and never open their UI, otherwise these function will not work as expected.")
	BeginFuncVA(getStack, "Get Stack", "Returns the item stack at the given index.\nTakes integers as input and returns the corresponding stacks.") {
	FuncBody()
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	int ArgNum = Params.Num();
	for (int i = 0; i < ArgNum; ++i) {
		const FINAny& Any = Params[i];
		FInventoryStack Stack;
		if (Any.GetType() == FIN_INT && self->GetStackFromIndex(Any.GetInt(), Stack)) { // GetInt realy correct?
			Params.Add(FINAny(Stack));
		} else {
			Params.Add(FINAny());
		}
	}
} EndFunc()
BeginProp(RInt, itemCount, "Item Count", "The absolute amount of items in the whole inventory.") {
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	Return (int64)self->GetNumItems(nullptr);
} EndProp()
BeginProp(RInt, size, "Size", "The count of available item stack slots this inventory has.") {
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	Return (int64)self->GetSizeLinear();
} EndProp()
BeginFunc(sort, "Sort", "Sorts the whole inventory. (like the middle mouse click into a inventory)") {
	FuncBody()
	UFINLogLibrary::Log(FIN_Log_Verbosity_Warning, TEXT("It is currently Unsafe/Buggy to call sort!"));
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	if (!self->IsLocked() && self->GetCanBeRearranged()) self->SortInventory();
} EndFunc()
BeginFunc(swapStacks, "Swap Stacks", "Swaps two given stacks inside the inventory.", 1) {
	InVal(RInt, index1, "Index 1", "The index of the first stack in the inventory.")
	InVal(RInt, index2, "Index 2", "The index of the second stack in the inventory.")
	OutVal(RBool, successful, "Successful", "True if the swap was successful.")
	FuncBody()
	UFINLogLibrary::Log(FIN_Log_Verbosity_Warning, TEXT("It is currently Unsafe/Buggy to call swapStacks!"));
	successful = UFGInventoryLibrary::MoveInventoryItem(self, index1, self, index2);
} EndFunc()
BeginFunc(flush, "Flush", "Removes all discardable items from the inventory completely. They will be gone! No way to get them back!", 0) {
	FuncBody()
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	TArray<FInventoryStack> stacks;
	self->GetInventoryStacks(stacks);
	self->Empty();
	for (const FInventoryStack& stack : stacks) {
		if (stack.HasItems() && stack.Item.IsValid() && !UFGItemDescriptor::CanBeDiscarded(stack.Item.GetItemClass())) {
			self->AddStack(stack);
		}
	}
} EndFunc()
EndClass()
