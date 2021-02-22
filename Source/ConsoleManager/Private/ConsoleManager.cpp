// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "ConsoleManager.h"
#include "ConsoleManagerStyle.h"
#include "ConsoleManagerCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SConsoleManagerSlateWidget.h"

#include "ISettingsModule.h"

#include "FileHelper.h"
#include "ConsoleManagerSettings.h"

#include "Dialogs/Dialogs.h"
#include "Dialogs/CustomDialog.h"
#include "ISettingsSection.h"

static const FName ConsoleManagerTabName("ConsoleManager");

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
	{
		TSharedPtr<ISettingsSection> Section = SettingsModule->RegisterSettings("Editor", "Plugins", "Console Manager",
			LOCTEXT("ConsoleManagerName", "Console Manager"),
			LOCTEXT("ConsoleManagerNameDesc",
				"Configure display options of commands"),
			GetMutableDefault<UConsoleManagerSettings>()
		);

		Section->OnModified().BindLambda([=]() {

			FLinearColor MatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->MatchingValuesColor;
			FLinearColor NotMatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->NotMatchingValuesColor;

			FConsoleManagerStyle::SetMatchingValuesColor(MatchingValuesColor);
			FConsoleManagerStyle::SetNotMachingValuesColor(NotMatchingValuesColor);
			
			if (ActiveTab.IsValid())
			{
				ActiveTab.Pin()->SetContent(BuildUI());
			}
			else
			{
				TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(ConsoleManagerTabName);
				if (Tab.IsValid())
				{
					Tab->SetContent(BuildUI());
				}
			}

			return true;
			});
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

	CommandsManager->Refresh();


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

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FConsoleManagerStyle::Shutdown();

	FConsoleManagerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ConsoleManagerTabName);

	CommandsManager.Reset();
}

void FConsoleManagerModule::OpenTab()
{
	ActiveTab = FGlobalTabmanager::Get()->InvokeTab(ConsoleManagerTabName);
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

	//If there isnt any group created on window open ask user if he wants to create new with all variables
	if (CommandsManager->GetCommandGroups().Num() == 0)
	{
		AskForDefaultGroup();
	}

	TSharedRef<SConsoleManagerSlateWidget> UI = BuildUI();

	FConsoleCommandDelegate Delegate;

	Delegate.BindLambda([=]() {
		CommandsManager->UpdateHistory();
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

	return DockTab;
}

TSharedRef<class SConsoleManagerSlateWidget> FConsoleManagerModule::BuildUI()
{
	bool DisplayCommandValueType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandValueType;
	bool DisplaySetByValue = GetMutableDefault<UConsoleManagerSettings>()->DisplaySetByValue;
	bool DisplayCommandType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandType;


	return SNew(SConsoleManagerSlateWidget)
		.CommandsManager(CommandsManager)
		.DisplayCommandValueType(DisplayCommandValueType)
		.DisplaySetByValue(DisplaySetByValue)
		.DisplayCommandType(DisplayCommandType);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleManagerModule, ConsoleManager)