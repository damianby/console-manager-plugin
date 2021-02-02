// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
#include "CommandsManager.h"

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;



class FConsoleManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	void OpenTab();

private:

	void RegisterMenus();

	void AskForDefaultGroup();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<class FUICommandList> PluginCommands;

	TSharedRef<class SConsoleManagerSlateWidget> BuildUI();

	TWeakPtr<SDockTab> ActiveTab;

	//This handle allows to catch any changes to cvars
	FConsoleVariableSinkHandle Handle;

	TSharedPtr<FCommandsManager> CommandsManager;
};
