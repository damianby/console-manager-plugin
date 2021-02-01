// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConsoleManager.h"
#include "ConsoleManagerStyle.h"
#include "ConsoleManagerCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SConsoleManagerSlateWidget.h"

#include "ISettingsModule.h"

#include "FileHelper.h"
#include "ConsoleManagerSettings.h"

static const FName ConsoleManagerTabName("ConsoleManager");

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
		SettingsModule->RegisterSettings("Editor", "Plugins", "Console Manager",
			LOCTEXT("ConsoleManagerName", "Console Manager"),
			LOCTEXT("ConsoleManagerNameDesc",
				"Configure display options of commands"),
			GetMutableDefault<UConsoleManagerSettings>()
		);
	}


	FConsoleManagerStyle::Initialize();
	FConsoleManagerStyle::ReloadTextures();

	FConsoleManagerCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FConsoleManagerCommands::Get().OpenTab,
		FExecuteAction::CreateRaw(this, &FConsoleManagerModule::OpenTab),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FConsoleManagerModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ConsoleManagerTabName,
		FOnSpawnTab::CreateRaw(
			this, &FConsoleManagerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FConsoleManagerTabTitle", "Console Manager"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	CommandsManager = TSharedPtr<FCommandsManager>(new FCommandsManager());


	FLinearColor MatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->MatchingValuesColor;
	FLinearColor NotMatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->NotMatchingValuesColor;

	FConsoleManagerStyle::SetMatchingValuesColor(MatchingValuesColor);
	FConsoleManagerStyle::SetNotMachingValuesColor(NotMatchingValuesColor);

}

void FConsoleManagerModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		SettingsModule->UnregisterSettings("Editor", "Plugins", "Console Manager");


	CommandsManager->SaveCommands();

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FConsoleManagerStyle::Shutdown();

	FConsoleManagerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ConsoleManagerTabName);

	CommandsManager.Reset();
}

void FConsoleManagerModule::OpenTab()
{
	FGlobalTabmanager::Get()->InvokeTab(ConsoleManagerTabName);
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

void FConsoleManagerModule::AskForDefaultGroup()
{
	TSharedRef<SWidget> ContentWidget =
		SNew(SBox)
		.Padding(FMargin(0.f, 15.f, 0.f, 0.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.f, 0.f, 0.f, 10.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("Would you like to create snapshot of all variables?"))))
		]
		];


	TSharedRef<SCustomDialog> EditDialog = SNew(SCustomDialog)
		.Title(FText(LOCTEXT("EditGroupDialog_Title", "Edit Group")))
		.DialogContent(ContentWidget)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});

	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = EditDialog->ShowModal();

	switch (ButtonPressed)
	{
	case 0:
		FCommandGroup* NewGroup = CommandsManager->AddNewGroup("Default");
		const FCommandGroup* AllCommands = CommandsManager->GetAllCommands();

		for (const FConsoleCommand& Command : AllCommands->Commands)
		{
			
			if (Command.GetObjType() == EConsoleCommandType::CVar)
			{
				FConsoleCommand& NewCommand = NewGroup->Commands.Add_GetRef(Command);
				NewCommand.SetValue(NewCommand.GetCurrentValue());
			}
		}

		break;
	}

}

TSharedRef<class SDockTab> FConsoleManagerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	CommandsManager->Refresh();


	TSharedRef<SConsoleManagerSlateWidget> UI = BuildUI();

	FConsoleCommandDelegate Delegate;

	Delegate.BindLambda([=]() {
		UI->RefreshListView();
		//CreateCommandList(ButtonList, AbsoluteFilePath);
		});

	Handle = IConsoleManager::Get().RegisterConsoleVariableSink_Handle(Delegate);

	TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			UI
		];


	SDockTab::FOnTabClosedCallback ClosedTabDelegate;

	ClosedTabDelegate.BindLambda([=](TSharedRef<SDockTab> DockTab)
		{
			IConsoleManager::Get().UnregisterConsoleVariableSink_Handle(Handle);
			CommandsManager->SaveCommands();
			DockTab->ClearContent();
		});

	DockTab->SetOnTabClosed(ClosedTabDelegate);


	//If there isnt any group created on window open ask user if he wants to create new with all variables
	if (CommandsManager->GetCommandGroups().Num() == 0)
	{
		AskForDefaultGroup();
	}


	return DockTab;
}

TSharedRef<class SConsoleManagerSlateWidget> FConsoleManagerModule::BuildUI()
{
	bool DisplayCommandValueType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandValueType;
	bool DisplaySetByValue = GetMutableDefault<UConsoleManagerSettings>()->DisplaySetByValue;


	return SNew(SConsoleManagerSlateWidget).CommandsManager(CommandsManager).DisplayCommandValueType(DisplayCommandValueType).DisplaySetByValue(DisplaySetByValue);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleManagerModule, ConsoleManager)