#include "FINLuaProcessorRewrite.h"

#include "AsyncWork.h"
#include "Base64.h"
#include "Engine/Engine.h"
#include "FGInventoryComponent.h"

#include "FicsItLogLibrary.h"
#include "FILLogContainer.h"
#include "FINComputerEEPROMDesc.h"
#include "FINItemStateEEPROMText.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua.h"
#include "FicsItKernel/FicsItKernel.h"
#include "FicsItNetworksLuaRewriteModule.h"

UFINLuaProcessorRewrite::UFINLuaProcessorRewrite() {}

void UFINLuaProcessorRewrite::BeginDestroy() {
	Runtime.Stop();
	Super::BeginDestroy();
}

void UFINLuaProcessorRewrite::GatherDependencies_Implementation(TArray<UObject*> &Out_DependentObjects) {
	Out_DependentObjects.Add(Kernel);
	//Out_DependentObjects.Add(AFINMediaSubsystem::GetMediaSubsystem(this));
}

void UFINLuaProcessorRewrite::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {
	// Runtime.PauseAndWait();
	// RuntimeState.Clear();
	// if (GetKernel()->GetState() != FIN_KERNEL_RUNNING)
	// 	return;
	// RuntimeState = Runtime.Runtime.SaveState();
	// if (RuntimeState.IsFailure()) {
	// 	FString message = FString::Printf(
	// 		TEXT(
	// 			"%s: Unable to save computer state into a save-file (computer will restart when loading the save-file): %s"),
	// 		*DebugInfo, *RuntimeState.Failure);
	// 	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s"), *message);
	// 	GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
	// }
}

void UFINLuaProcessorRewrite::PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {}

void UFINLuaProcessorRewrite::PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) {}

void UFINLuaProcessorRewrite::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) {
	// Runtime.Stop();
	// Runtime.Start();
}

void UFINLuaProcessorRewrite::SetKernel(UFINKernelSystem *InKernel) {
	//if (GetKernel() && GetKernel()->GetFileSystem()) GetKernel()->GetFileSystem()->removeListener(FileSystemListener);
	Kernel = InKernel;
}

void UFINLuaProcessorRewrite::Tick(float InDelta) {
	// if (GetKernel()->GetNetwork()->GetSignalCount() > 0) {
	// 	Runtime.Runtime.Timeout.Reset();
	// }

	// Runtime.Runtime.Hook_Tick = 2500;
	Runtime.Tick();

	switch (Runtime.GetStatus()) {
		case FFINLuaThread::EState::Finished:
			GetKernel()->Stop();
			break;
		case FFINLuaThread::EState::Crashed:
			GetKernel()->Crash(MakeShared<FFINKernelCrash>(**Runtime.GetError()));
			break;
		default:
			break;
	}
}

void UFINLuaProcessorRewrite::Stop(const bool bIsCrash) {
	UE_LOG(LogFicsItNetworksLuaRewrite, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo,
	       bIsCrash ? TEXT("due to crash") : TEXT(""));
	Runtime.Stop();
}

void UFINLuaProcessorRewrite::Reset() {
	//UObject *Comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
	//AFINSignalSubsystem::GetSignalSubsystem(Comp)->IgnoreAll(Comp);
	Kernel->GetNetwork()->ClearSignals();

	//RuntimeState.Clear();

	Runtime.Stop();
	Runtime.SetCode(GetEEPROM());
	Runtime.Start();
}

TOptional<FString> UFINLuaProcessorRewrite::GetEEPROM() const {
	const FInventoryItem EEPROM = Kernel->GetEEPROM();
	if (const FFINItemStateEEPROMText *State = EEPROM.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		return State->Code;
	}
	return {};
}

bool UFINLuaProcessorRewrite::SetEEPROM(const FString &Code) {
	FInventoryItem EEPROM = Kernel->GetEEPROM();
	UFINComputerEEPROMDesc::CreateEEPROMStateInItem(EEPROM);

	if (const FFINItemStateEEPROMText *StateLua = EEPROM.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		FFINItemStateEEPROMText State = *StateLua;
		State.Code = Code;
		return Kernel->SetEEPROM(FFGDynamicStruct(State));
	}

	return false;
}
