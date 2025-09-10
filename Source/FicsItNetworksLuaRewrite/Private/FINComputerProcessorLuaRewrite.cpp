#include "FINComputerProcessorLuaRewrite.h"
#include "FINLuaProcessorRewrite.h"

UFINKernelProcessor* AFINComputerProcessorLuaRewrite::CreateProcessor() {
	const auto Processor = NewObject<UFINLuaProcessorRewrite>(this);
	Processor->DebugInfo = this->GetName();
	return Processor;
}
