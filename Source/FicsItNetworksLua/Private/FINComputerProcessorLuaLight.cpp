#include "FINComputerProcessorLuaLight.h"
#include "FINLuaProcessorLight.h"

UFINKernelProcessor* AFINComputerProcessorLuaLight::CreateProcessor() {
	const auto Processor = NewObject<UFINLuaProcessorLight>(this);
	Processor->DebugInfo = this->GetName();
	return Processor;
}
