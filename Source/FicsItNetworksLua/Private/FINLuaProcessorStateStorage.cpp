#include "FINLuaProcessorStateStorage.h"

#include "FicsItNetworksLuaModule.h"
#include "FicsItNetworksModule.h"
#include "Network/FINDynamicStructHolder.h"
#include "Utils/FINUtils.h"

bool FFINLuaProcessorStateStorage::Serialize(FStructuredArchive::FSlot Slot) {
	if (!Slot.GetUnderlyingArchive().IsSaveGame()) return false;
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(SA_FIELD_NAME(TEXT("Traces"))).GetUnderlyingArchive() << Traces;
	Record.EnterField(SA_FIELD_NAME(TEXT("References"))) << References;
	Record.EnterField(SA_FIELD_NAME(TEXT("Thread"))) << LuaData;
	FString Str;
	Record.EnterField(SA_FIELD_NAME(TEXT("Globals"))) << Str;

	FVersion version = UFINUtils::GetFINSaveVersion(GWorld);
	if (FVersion(0, 3, 19).Compare(version) == 1) return false;

	int32 StructNum = Structs.Num();
	FStructuredArchiveArray Array = Record.EnterArray(SA_FIELD_NAME(TEXT("Structs")), StructNum);
	
	if (Record.GetUnderlyingArchive().IsLoading()) Structs.Empty();
	for (int i = 0; i < StructNum; ++i) {
		if (Record.GetUnderlyingArchive().IsLoading()) Structs.Add(MakeShared<FFINDynamicStructHolder>());
		TSharedPtr<FFINDynamicStructHolder> holder = Structs[i];
		if (holder) {
			holder->Serialize(Array.EnterElement());
		} else {
			FFINDynamicStructHolder().Serialize(Array.EnterElement());
		}
	}
	return true;
}

int32 FFINLuaProcessorStateStorage::Add(const FFINNetworkTrace& Trace) {
	return Traces.AddUnique(Trace);
}

int32 FFINLuaProcessorStateStorage::Add(UObject* Ref) {
	return References.AddUnique(Ref);
}

int32 FFINLuaProcessorStateStorage::Add(TSharedPtr<FFINDynamicStructHolder> Struct) {
	return Structs.Add(Struct);
}

FFINNetworkTrace FFINLuaProcessorStateStorage::GetTrace(int32 id) {
	return Traces[id];
}

UObject* FFINLuaProcessorStateStorage::GetRef(int32 id) {
	return References[id];
}

TSharedPtr<FFINDynamicStructHolder> FFINLuaProcessorStateStorage::GetStruct(int32 id) {
	if (id >= Structs.Num()) {
		UE_LOG(LogFicsItNetworksLua, Warning, TEXT("Unable to find struct in lua processor state storage with id %i"), id);
		return MakeShared<FFINDynamicStructHolder>();
	}
	return Structs[id];
}

void FFINLuaProcessorStateStorage::Clear() {
	Traces.Empty();
	References.Empty();
	Structs.Empty();
	LuaData.Empty();
}
