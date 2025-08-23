#include "FINLuaProcessorLight.h"

#include "AsyncWork.h"
#include "Base64.h"
#include "FGInventoryComponent.h"
#include "FicsItLogLibrary.h"
#include "FicsItNetworksLuaModule.h"
#include "FILLogContainer.h"
#include "FINComputerEEPROMDesc.h"
#include "FINItemStateEEPROMText.h"
#include "FINMediaSubsystem.h"
#include "FINSignalSubsystem.h"
#include "LuaEventAPI.h"
#include "LuaFuture.h"
#include "LuaKernelAPI.h"
#include "LuaWorldAPI.h"
#include "Engine/Engine.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FINLua/LuaUtil.h"
#include "Signals/FINSignalData.h"

UFINLuaProcessorLight::UFINLuaProcessorLight() {
	// Runtime.Runtime.Modules.Add("DebugModule");
	// Runtime.Runtime.Modules.Add("LogModule");
	// Runtime.Runtime.Modules.Add("KernelModule");
	// Runtime.Runtime.Modules.Add("ComputerModule");
	// Runtime.Runtime.Modules.Add("ComponentModule");
	// Runtime.Runtime.Modules.Add("WorldModule");
	// Runtime.Runtime.Modules.Add("EventModule");
	// Runtime.Runtime.Modules.Add("FutureModule");
	// Runtime.Runtime.Modules.Add("FileSystemModule");
	// Runtime.Runtime.OnPreLuaTick.AddWeakLambda(this, [this](TArray<TSharedPtr<void>>& TickStack) {
	// 	TickStack.Add(MakeShared<FFILLogScope>(GetKernel()->GetLog()));
	// });
	// Runtime.Runtime.OnPreModules.AddWeakLambda(this, [this]() {
	// 	FINLua::luaFIN_setReferenceCollector(Runtime.Runtime.GetLuaState(), ReferenceCollector);
	// 	FINLua::luaFIN_setWorld(Runtime.Runtime.GetLuaState(), GetWorld());
	// 	FINLua::luaFIN_setComponentNetwork(Runtime.Runtime.GetLuaState(), &ComponentNetwork);
	//  FINLua::luaFIN_setKernel(Runtime.Runtime.GetLuaState(), GetKernel());
	// 	FINLua::luaFIN_setFileSystem(Runtime.Runtime.GetLuaState(), GetKernel()->GetFileSystem());
	// 	FINLua::luaFIN_setEventSystem(Runtime.Runtime.GetLuaState(), EventSystem);
	// 	FINLua::luaFIN_createFutureDelegate(Runtime.Runtime.GetLuaState()).AddWeakLambda(this, [this](const FINLua::FLuaFuture& Future) {
	// 		GetKernel()->PushFuture(Future);
	// 	});
	// });
	// Runtime.Runtime.OnPostReset.AddWeakLambda(this, [this]() {
	// 	TOptional<FString> error = Runtime.Runtime.LoadState(RuntimeState);
	// 	if (error) {
	// 		GetKernel()->Reset();
	// 		FString message = FString::Printf(TEXT("%s: Unable to load computer state from save-file (computer will restart): %s"), *DebugInfo, **error);
	// 		UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s"), *message);
	// 		GetKernel()->GetLog()->PushLogEntry(FIL_Verbosity_Warning, message);
	// 	}
	// });

	ComponentNetwork.OnGetComponentByID.BindWeakLambda(this, [this](const FGuid &ID) {
		return GetKernel()->GetNetwork()->GetComponentByID(ID);
	});
	ComponentNetwork.OnGetComponentByNick.BindWeakLambda(this, [this](const FString &Nick) {
		return GetKernel()->GetNetwork()->GetComponentByNick(Nick);
	});
	ComponentNetwork.OnGetComponentByClass.BindWeakLambda(this, [this](UClass *Class, const bool bInRedirect) {
		return GetKernel()->GetNetwork()->GetComponentByClass(Class, bInRedirect);
	});

	EventSystem.OnTimeSinceStart.BindWeakLambda(this, [this]() {
		return GetKernel()->GetTimeSinceStart();
	});
	EventSystem.OnListen.BindWeakLambda(this, [this](const FFIRTrace &Object) {
		const UFINKernelSystem *Kernel = GetKernel();
		const UFINKernelNetworkController *Network = Kernel->GetNetwork();
		AFINSignalSubsystem::GetSignalSubsystem(GetKernel())->Listen(Object.GetUnderlyingPtr(),
		                                                             Object.Reverse() / Network->GetComponent().
		                                                             GetObject());
	});
	EventSystem.OnListening.BindWeakLambda(this, [this]() {
		UObject *Comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		return AFINSignalSubsystem::GetSignalSubsystem(Comp)->GetListening(Comp);
	});
	EventSystem.OnIgnoreAll.BindWeakLambda(this, [this]() {
		UObject *Comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		AFINSignalSubsystem::GetSignalSubsystem(Comp)->IgnoreAll(Comp);
	});
	EventSystem.OnClear.BindWeakLambda(this, [this]() {
		GetKernel()->GetNetwork()->ClearSignals();
	});
	EventSystem.OnIgnore.BindWeakLambda(this, [this](UObject *Object) {
		UObject *Comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
		AFINSignalSubsystem::GetSignalSubsystem(Comp)->Ignore(Comp, Object);
	});
	EventSystem.OnPullSignal.BindWeakLambda(this, [this]() -> TOptional<TTuple<FFIRTrace, FFINSignalData>> {
		if (GetKernel()->GetNetwork()->GetSignalCount() < 1) {
			return {};
		}
		FFIRTrace Sender;
		FFINSignalData Data = GetKernel()->GetNetwork()->PopSignal(Sender);
		return {{Sender, Data}};
	});
}

void UFINLuaProcessorLight::BeginDestroy() {
	Runtime.Stop();
	Super::BeginDestroy();
}

void UFINLuaProcessorLight::GatherDependencies_Implementation(TArray<UObject*> &Out_DependentObjects) {
	Out_DependentObjects.Add(Kernel);
	Out_DependentObjects.Add(AFINMediaSubsystem::GetMediaSubsystem(this));
}

void UFINLuaProcessorLight::PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {
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

void UFINLuaProcessorLight::PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) {}

void UFINLuaProcessorLight::PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) {}

void UFINLuaProcessorLight::PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) {
	// Runtime.Stop();
	// Runtime.Start();
}

void UFINLuaProcessorLight::SetKernel(UFINKernelSystem *InKernel) {
	//if (GetKernel() && GetKernel()->GetFileSystem()) GetKernel()->GetFileSystem()->removeListener(FileSystemListener);
	Kernel = InKernel;
}

void UFINLuaProcessorLight::Tick(float InDelta) {
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

void UFINLuaProcessorLight::Stop(const bool bIsCrash) {
	UE_LOG(LogFicsItNetworksLua, Display, TEXT("%s: Lua Processor stop %s"), *DebugInfo,
	       bIsCrash ? TEXT("due to crash") : TEXT(""));
	Runtime.Stop();
}

void UFINLuaProcessorLight::Reset() {
	UObject *Comp = GetKernel()->GetNetwork()->GetComponent().GetObject();
	AFINSignalSubsystem::GetSignalSubsystem(Comp)->IgnoreAll(Comp);
	Kernel->GetNetwork()->ClearSignals();

	RuntimeState.Clear();

	Runtime.Stop();
	Runtime.SetCode(GetEEPROM());
	Runtime.Start();
}

TOptional<FString> UFINLuaProcessorLight::GetEEPROM() const {
	const FInventoryItem EEPROM = Kernel->GetEEPROM();
	if (const FFINItemStateEEPROMText *State = EEPROM.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		return State->Code;
	}
	return {};
}

bool UFINLuaProcessorLight::SetEEPROM(const FString &Code) {
	FInventoryItem EEPROM = Kernel->GetEEPROM();
	UFINComputerEEPROMDesc::CreateEEPROMStateInItem(EEPROM);

	if (const FFINItemStateEEPROMText *StateLua = EEPROM.GetItemState().GetValuePtr<FFINItemStateEEPROMText>()) {
		FFINItemStateEEPROMText State = *StateLua;
		State.Code = Code;
		return Kernel->SetEEPROM(FFGDynamicStruct(State));
	}

	return false;
}
