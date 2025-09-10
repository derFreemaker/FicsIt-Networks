#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLuaRewrite, Verbose, All);

class FFicsItNetworksLuaRewriteModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};
