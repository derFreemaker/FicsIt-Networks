#pragma once

#include "FINComputerProcessorLua.h"

#include "FINComputerProcessorLuaLight.generated.h"

UCLASS(Blueprintable)
class FICSITNETWORKSLUA_API AFINComputerProcessorLuaLight : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	int LuaInstructionsPerTick = 1;
	
	// Begin AFINComputerProcessorLua
	virtual UFINKernelProcessor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};