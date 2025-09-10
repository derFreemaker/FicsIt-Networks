#pragma once

#include "CoreMinimal.h"
#include "ComputerModules/FINComputerProcessor.h"

#include "FINComputerProcessorLuaRewrite.generated.h"

UCLASS(Blueprintable)
class FICSITNETWORKSLUAREWRITE_API AFINComputerProcessorLuaRewrite : public AFINComputerProcessor {
	GENERATED_BODY()
public:
	// Begin AFINComputerProcessorLua
	virtual UFINKernelProcessor* CreateProcessor() override;
	// End AFINComputerProcessorLua
};