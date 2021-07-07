// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
#include "CommandsManager.h"

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class UCommandsContainer;


class FConsoleManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OpenTab();

	void OpenSettings();

	void OpenTabs(const TArray<UCommandsContainer*> Containers);
	void OpenTabLast();

	/**
	 * @brief Provide singleton like access to this module
	 * @return Module
	*/
	static inline FConsoleManagerModule& GetModule()
	{
		static const FName ModuleName = "ConsoleManager";
		return FModuleManager::LoadModuleChecked< FConsoleManagerModule >(ModuleName);
	}

	FCommandsManager* GetCommandsManager() { return CommandsManager.Get(); }

private:

	void ApplySettings();
	void RegisterMenus();

	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	TSharedRef<SWidget> GenerateMenuContent();

	void SpawnNewContainer();
	UCommandsContainer* CreateNewContainer();

	void InitializeCommandsManager(const TArray<UCommandsContainer*>* Containers = nullptr);

	void SpawnOrActivateTab();
	void SaveLastSelectedObjects();

	TArray<UCommandsContainer*> LoadAllContainers();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<class FUICommandList> PluginCommands;

	TSharedRef<class SConsoleManagerSlateWidget> BuildUI();

	TWeakPtr<class SConsoleManagerSlateWidget> ActiveTab;

	TSharedPtr<FCommandsManager> CommandsManager;

	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	bool bIsTabAutostarted = true;

	TWeakPtr<FTabManager> LastTabManager;

	TSharedPtr<FExtender> ToolBarExtender;
};
