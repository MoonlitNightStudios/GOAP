// Copyright Epic Games, Inc. All Rights Reserved.

#include "GOAPDebug.h"

#include "GameplayDebugger.h"
#include "GOAPAgentDebuggerCategory.h"

#define LOCTEXT_NAMESPACE "FGOAPDebugModule"

void FGOAPDebugModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	IModuleInterface::StartupModule();

	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory("GOAP", IGameplayDebugger::FOnGetCategory::CreateStatic(&FGOAPAgentDebuggerCategory::MakeInstance), EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
}

void FGOAPDebugModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGOAPDebugModule, GOAPDebug)
