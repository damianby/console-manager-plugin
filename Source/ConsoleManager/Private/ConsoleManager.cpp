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

#include "CommandsContainer.h"
#include "Engine/AssetManager.h"
#include "CommandsContainerActions.h"
#include "CommandsContainer.h"


#include "ISettingsContainer.h"
#include "ISettingsCategory.h"

#include "LevelEditor.h"
#include "SDeviceProfileCreateProfilePanel.h"

static const FName ConsoleManagerTabName("ConsoleManager");

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"


void FConsoleManagerModule::StartupModule()
{
	CommandsManager = TSharedPtr<FCommandsManager>(new FCommandsManager());
	
	UE_LOG(LogTemp, Warning, TEXT("STARTUP MODULE"));

	//FCoreDelegates::OnPostEngineInit
	// We load module after engine and editor init so everything should be ready
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) 
	{
		TSharedPtr<ISettingsSection> Section = SettingsModule->RegisterSettings("Editor", "Plugins", "Console Manager",
			LOCTEXT("ConsoleManagerName", "Console Manager"),
			LOCTEXT("ConsoleManagerNameDesc",
				"Configure display options of commands"),
			GetMutableDefault<UConsoleManagerSettings>()
		);
		
		// Rebuild UI if active after change to settings
		Section->OnModified().BindLambda([=]() {

			if (!ActiveTab.IsValid())
			{
				TSharedPtr<SDockTab> FoundDockTab = FGlobalTabmanager::Get()->FindExistingLiveTab(ConsoleManagerTabName);
				if (FoundDockTab.IsValid())
				{
					TSharedPtr<SConsoleManagerSlateWidget> Tab = StaticCastSharedPtr< SConsoleManagerSlateWidget, SDockTab>(FoundDockTab);
					if (Tab.IsValid())
					{
						ActiveTab = Tab;
					}
				}
			}
			
			CommandsManager->ShouldLoadAllContainers(GetMutableDefault<UConsoleManagerSettings>()->StartupOption == EConsoleManagerStartupOption::AllContainers);
			


			ApplySettings();

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


	// Register in level editor module for global shortcut
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FConsoleManagerModule::RegisterMenus));


	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ConsoleManagerTabName,
		FOnSpawnTab::CreateRaw(
			this, &FConsoleManagerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FConsoleManagerTabTitle", "Console Manager"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);


	// Register actions Commands Container asset
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<FCommandsContainerActions> Actions = MakeShared<FCommandsContainerActions>();
	AssetTools.RegisterAssetTypeActions(Actions);
	RegisteredAssetTypeActions.Add(Actions);


	ApplySettings();
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

	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

	if (AssetToolsModule != nullptr)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

void FConsoleManagerModule::OpenTab()
{
	bIsTabAutostarted = false;

	UE_LOG(LogTemp, Warning, TEXT("Open tab called!"));


	if (ActiveTab.IsValid())
	{
		ActiveTab.Pin()->DrawAttention();
		ActiveTab.Pin()->TabActivated();
	}
	else
	{
		/*TSharedPtr<SDockTab> TabAlive = FGlobalTabmanager::Get()->FindExistingLiveTab(ConsoleManagerTabName);

		if (TabAlive.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Tab is alive!"));

			TSharedPtr<SConsoleManagerSlateWidget> CMWidget = StaticCastSharedPtr<SConsoleManagerSlateWidget>(TabAlive);
			if (CMWidget.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("CM is valid"));
				FGlobalTabmanager::Get()->TryInvokeTab(ConsoleManagerTabName);
				CMWidget->TabActivated();
			}
		}*/

		UE_LOG(LogTemp, Warning, TEXT("Tab alive is not found!"));

		EConsoleManagerStartupOption StartupOption = GetMutableDefault<UConsoleManagerSettings>()->StartupOption;

		switch (StartupOption)
		{
		case EConsoleManagerStartupOption::LastOpened:
		{
			TArray<TSoftObjectPtr<UCommandsContainer>> LastOpenedObjs = GetMutableDefault<UConsoleManagerSettings>()->LastSelectedObjs;

			TArray<UCommandsContainer*> Objects;

			for (auto& SoftObjPtr : LastOpenedObjs)
			{
				
				UCommandsContainer* ExistingObj = SoftObjPtr.Get();
				if (ExistingObj)
				{
					Objects.Add(ExistingObj);
				}
				else
				{
					UCommandsContainer* LoadedObj = SoftObjPtr.LoadSynchronous();
					Objects.Add(LoadedObj);
				}
			}

			CommandsManager->Initialize(Objects);

			break;
		}
		case EConsoleManagerStartupOption::Specified:
		{
			UCommandsContainer* LoadedAsset = GetMutableDefault<UConsoleManagerSettings>()->AssetToLoad.Get();

			if (LoadedAsset)
			{
				CommandsManager->Initialize(TArray<UCommandsContainer*>{LoadedAsset});

			}
			else
			{
				// Loads all if specified asset is not found
				CommandsManager->Initialize();
			}

			break;
		}
		case EConsoleManagerStartupOption::AllContainers:
		default:

			CommandsManager->Initialize();

			break;
		}
		
		if (LastTabManager.IsValid()) {
			LastTabManager.Pin()->TryInvokeTab(ConsoleManagerTabName);
		}
		else {
			FGlobalTabmanager::Get()->TryInvokeTab(ConsoleManagerTabName);
		}
	}
}

void FConsoleManagerModule::OpenSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	SettingsModule->ShowViewer("Editor", "Plugins", "Console Manager");
}

void FConsoleManagerModule::OpenTab(const TArray<UObject*>& Containers)
{
	bIsTabAutostarted = false;

	TArray<UCommandsContainer*> OutCommandsContainers;

	for (const auto& Container : Containers)
	{
		UCommandsContainer* OutContainer = StaticCast<UCommandsContainer*>(Container);
		OutCommandsContainers.Add(OutContainer);
	}

	CommandsManager->Initialize(OutCommandsContainers);

	if (ActiveTab.IsValid())
	{
		ActiveTab.Pin()->TabActivated();
	}

	if (LastTabManager.IsValid()) {
		LastTabManager.Pin()->TryInvokeTab(ConsoleManagerTabName);
	}
	else {
		FGlobalTabmanager::Get()->TryInvokeTab(ConsoleManagerTabName);
	}

	//TSharedPtr<SDockTab> TabAlive = FGlobalTabmanager::Get()->TryInvokeTab(ConsoleManagerTabName);

	//if (TabAlive.IsValid())
	//{
	//	TSharedPtr<SConsoleManagerSlateWidget> CMWidget = StaticCastSharedPtr<SConsoleManagerSlateWidget>(TabAlive);
	//	if (CMWidget.IsValid())
	//	{
	//		CMWidget->TabActivated();
	//	}
	//}
}

void FConsoleManagerModule::ApplySettings()
{
	const FLinearColor& MatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->MatchingValuesColor;
	const FLinearColor& NotMatchingValuesColor = GetMutableDefault<UConsoleManagerSettings>()->NotMatchingValuesColor;
	int32 CommandsFontSize = GetMutableDefault<UConsoleManagerSettings>()->CommandsFontSize;

	FConsoleManagerStyle::SetMatchingValuesColor(MatchingValuesColor);
	FConsoleManagerStyle::SetNotMachingValuesColor(NotMatchingValuesColor);
	FConsoleManagerStyle::SetCommandsFontSize(CommandsFontSize);

	FInputChord OpenManagerShortcut = GetMutableDefault<UConsoleManagerSettings>()->OpenShortcut;
	if (OpenManagerShortcut.IsValidChord())
	{
		FConsoleManagerCommands::Get().OpenTab->SetActiveChord(OpenManagerShortcut, EMultipleKeyBindingIndex::Secondary);
	}

	if (CommandsManager.IsValid())
	{
		CommandsManager->SetHistoryBufferSize(GetMutableDefault<UConsoleManagerSettings>()->HistoryBufferSize);
	}

	if (ActiveTab.IsValid())
	{
		const bool DisplayCommandValueType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandValueType;
		const bool DisplaySetByValue = GetMutableDefault<UConsoleManagerSettings>()->DisplaySetByValue;
		const bool DisplayCommandType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandType;

		ActiveTab.Pin()->UpdateHeaderColumnsVisibility(DisplayCommandValueType, DisplaySetByValue, DisplayCommandType);
	}

	CommandsManager->ShouldLoadAllContainers(GetMutableDefault<UConsoleManagerSettings>()->StartupOption == EConsoleManagerStartupOption::AllContainers);
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

				//Section.AddEntry(FToolMenuEntry::InitComboButton(
				//	"ConsoleManagerDropdown",
				//	FUIAction(
				//		FExecuteAction(),
				//		FCanExecuteAction(),
				//		FIsActionChecked(),
				//		FIsActionButtonVisible::CreateStatic(FLevelEditorActionCallbacks::CanShowSourceCodeActions)),
				//	FNewToolMenuChoice(),
				//	LOCTEXT("CompileCombo_Label", "Compile Options"),
				//	LOCTEXT("CompileComboToolTip", "Compile options menu"),
				//	FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Recompile"),
				//	true
				//));

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
		

		//FCommandGroup* NewGroup = CommandsManager->AddNewGroup("Default");
		//const FCommandGroup* AllCommands = CommandsManager->GetAllCommands();

		//for (const FConsoleCommand& Command : AllCommands->Commands)
		//{
		//	
		//	if (Command.GetObjType() == EConsoleCommandType::CVar)
		//	{
		//		FConsoleCommand& NewCommand = NewGroup->Commands.Add_GetRef(Command);
		//		NewCommand.SetValue(NewCommand.GetCurrentValue());
		//	}
		//}

		break;
	}
}

TSharedRef<class SDockTab> FConsoleManagerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{

	//TSharedRef<SDockTab> Tab = SNew(SDockTab)
	//	.TabRole(ETabRole::NomadTab);


	//TWeakObjectPtr<UDeviceProfileManager> DevProf = TWeakObjectPtr<UDeviceProfileManager>(&UDeviceProfileManager::Get());

	//TSharedRef<SWidget> ProfCr = SNew(SDeviceProfileCreateProfilePanel, DevProf);



	//Tab->SetContent(ProfCr);


	//return Tab;





	//If there isnt any group created on window open ask user if he wants to create new with all variables
	//if (CommandsManager->GetCommandGroups().Num() == 0)
	//{
	//	AskForDefaultGroup();
	//}

	// if its autostarted then we load previously opened containers
	if (bIsTabAutostarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("IS AUTOSTARTED"));

		TArray<TSoftObjectPtr<UCommandsContainer>> LastOpenedObjs = GetMutableDefault<UConsoleManagerSettings>()->LastSelectedObjs;
		TArray<UCommandsContainer*> Objects;

		UE_LOG(LogTemp, Warning, TEXT("Last opened objects num: %d"), LastOpenedObjs.Num());

		for (auto& SoftObjPtr : LastOpenedObjs)
		{
			
			UCommandsContainer* ExistingObj = SoftObjPtr.Get();
			if (ExistingObj)
			{
				Objects.Add(ExistingObj);
				UE_LOG(LogTemp, Warning, TEXT("Last object: %s"), *ExistingObj->GetName());
			}
			else
			{
				UCommandsContainer* LoadedObj = SoftObjPtr.LoadSynchronous();
				if (LoadedObj)
				{
					Objects.Add(LoadedObj);
					UE_LOG(LogTemp, Warning, TEXT("Loading assets"));
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("OBJS NUM after: %d"), Objects.Num());
		// If there are no previously selected we load all containers
		if (Objects.Num() > 0)
		{

			CommandsManager->Initialize(Objects);
		}
		else
		{
			CommandsManager->Initialize();
		}

	}


	TSharedRef<SConsoleManagerSlateWidget> UI = BuildUI();
	
	ActiveTab = UI;

	return UI;
}

TSharedRef<class SConsoleManagerSlateWidget> FConsoleManagerModule::BuildUI()
{
	const bool DisplayCommandValueType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandValueType;
	const bool DisplaySetByValue = GetMutableDefault<UConsoleManagerSettings>()->DisplaySetByValue;
	const bool DisplayCommandType = GetMutableDefault<UConsoleManagerSettings>()->DisplayCommandType;

	TSharedRef<SConsoleManagerSlateWidget> ConsoleManagerSlate = SNew(SConsoleManagerSlateWidget)
		.CommandsManager(CommandsManager)
		.DisplayCommandValueType(DisplayCommandValueType)
		.DisplaySetByValue(DisplaySetByValue)
		.DisplayCommandType(DisplayCommandType);


	SDockTab::FOnTabClosedCallback ClosedTabDelegate;

	ClosedTabDelegate.BindLambda([this](TSharedRef<SDockTab> DockTab)
		{

			const TArray<UCommandsContainer*>& Containers = CommandsManager->GetCommandsContainers();

			ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
			TArray< TSoftObjectPtr<UCommandsContainer>> LastSelectedObjs;

			for (auto& Container : Containers)
			{
				LastSelectedObjs.Add(TSoftObjectPtr<UCommandsContainer>(Container));
			}

			GetMutableDefault<UConsoleManagerSettings>()->LastSelectedObjs = LastSelectedObjs;
			auto Section = SettingsModule->GetContainer("Editor")->GetCategory("Plugins")->GetSection("Console Manager");
			Section->Save();

			UE_LOG(LogTemp, Warning, TEXT("Closed window test"));

			//CommandsManager->SaveToAssets();

			LastTabManager = ActiveTab.Pin()->GetTabManager();
			
			ActiveTab.Reset();
		});

	ConsoleManagerSlate->SetOnTabClosed(ClosedTabDelegate);

	return ConsoleManagerSlate;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleManagerModule, ConsoleManager)