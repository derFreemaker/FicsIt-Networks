#include "FicsItNetworksLuaRewriteModule.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaRewrite);

IMPLEMENT_GAME_MODULE(FFicsItNetworksLuaRewriteModule, FicsItNetworksLuaRewrite);

void FFicsItNetworksLuaRewriteModule::StartupModule() {
//	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
//#if !WITH_EDITOR
//		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameMode::PostLogin, (void*)GetDefault<AFGGameMode>(), [](AFGGameMode* gm, APlayerController* pc) {
//			if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
//				gm->RegisterRemoteCallObjectClass(UFINLuaRCO::StaticClass());
//			}
//		});
//#endif
//	});
}

void FFicsItNetworksLuaRewriteModule::ShutdownModule() {
}
