#pragma once

#include "CoreMinimal.h"
#include "AsyncWork.h"
#include "Optional.h"

#include "FicsItFileSystem/Listener.h"
#include "FicsItKernel/Processor/Processor.h"
#include "FINLuaRuntimeRewrite.h"

#include "FINLuaProcessorRewrite.generated.h"

UCLASS()
class FICSITNETWORKSLUAREWRITE_API UFINLuaProcessorRewrite : public UFINKernelProcessor {
	GENERATED_BODY()
	
//	UPROPERTY()
//	FFINLuaReferenceCollector ReferenceCollector;
	
	FFINLuaRuntimeRewrite Runtime = {*this};
//	FFINLuaComponentNetworkAccessDelegates ComponentNetwork;
//	FFINLuaEventSystem EventSystem;

public:
//	UPROPERTY(SaveGame)
//	FFINLuaRuntimePersistenceState RuntimeState;

	UFINLuaProcessorRewrite();

	// Begin UObject
	virtual void BeginDestroy() override;
	// End UObject

	// Begin IFGSaveInterface
	virtual void GatherDependencies_Implementation(TArray<UObject*> &Out_DependentObjects) override;
	virtual void PreSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostSaveGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PreLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	virtual void PostLoadGame_Implementation(int32 SaveVersion, int32 GameVersion) override;
	// End IFGSaveInterface

	// Begin Processor
	virtual void SetKernel(UFINKernelSystem *InKernel) override;
	virtual void Tick(float InDelta) override;
	virtual void Stop(bool bIsCrash) override;
	virtual void Reset() override;
	// End Processor

	/**
	 * Allows to access the eeprom code used by the processor.
	 * None if no eeprom is currently set.
	 *
	 * @return the eeprom code used by the processor.
	 */
	TOptional<FString> GetEEPROM() const;

	/**
	 * Allows to change the EEPROMs Code.
	 *
	 * @return true if a compatible eeprom is available to store the given code.
	 */
	bool SetEEPROM(const FString &Code);

	/**
	 * Tries to pop a signal from the signal queue in the network controller
	 * and pushes the resulting values to the given lua stack.
	 *
	 * @param[in]	L	the stack were the values should get pushed to.
	 * @return	the count of values we have pushed.
	 */
	int DoSignal(lua_State *L);
};
