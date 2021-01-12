// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConsoleManager.h"
#include "ConsoleManagerStyle.h"
#include "ConsoleManagerCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName ConsoleManagerTabName("ConsoleManager");

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FConsoleManagerStyle::Initialize();
	FConsoleManagerStyle::ReloadTextures();

	FConsoleManagerCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FConsoleManagerCommands::Get().OpenTab,
		FExecuteAction::CreateRaw(this, &FConsoleManagerModule::OpenTab),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FConsoleManagerModule::RegisterMenus));
}

void FConsoleManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FConsoleManagerStyle::Shutdown();

	FConsoleManagerCommands::Unregister();
}


void FConsoleManagerModule::OpenTab()
{
	UE_LOG(LogTemp, Warning, TEXT("Tab open!"));
}

void FConsoleManagerModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FConsoleManagerCommands::Get().OpenTab, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FConsoleManagerCommands::Get().OpenTab));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleManagerModule, ConsoleManager)