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
#include "Framework/Notifications/NotificationManager.h"

#include "ISettingsContainer.h"
#include "ISettingsCategory.h"

#include "CommandsContainerFactoryNew.h"
#include "Interfaces/IMainFrameModule.h"

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
		Section->OnModified().BindLambda([this]() {

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

	PluginCommands->MapAction(
		FConsoleManagerCommands::Get().OpenTabLast,
		FExecuteAction::CreateRaw(this, &FConsoleManagerModule::OpenTabLast),
		FCanExecuteAction());

	// Register in level editor module for global shortcut
	/*FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());*/

	// Register globally instead of level editor
	IMainFrameModule& MainFrame = FModuleManager::Get().LoadModuleChecked<IMainFrameModule>("MainFrame");
	MainFrame.GetMainFrameCommandBindings()->Append(PluginCommands.ToSharedRef());


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

	if (!IsEngineExitRequested() && ToolBarExtender.IsValid())
	{
		FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditorModule)
		{
			LevelEditorModule->GetToolBarExtensibilityManager()->RemoveExtender(ToolBarExtender);
		}
	}

}

void FConsoleManagerModule::OpenTab()
{
	bIsTabAutostarted = false;

	InitializeCommandsManager();
	SpawnOrActivateTab();
}

void FConsoleManagerModule::OpenTabs(const TArray<UCommandsContainer*> Containers)
{
	bIsTabAutostarted = false;

	InitializeCommandsManager(&Containers);
	SpawnOrActivateTab();
}

void FConsoleManagerModule::OpenTabLast()
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

	InitializeCommandsManager(&Objects);
	SpawnOrActivateTab();
}

void FConsoleManagerModule::OpenSettings()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	SettingsModule->ShowViewer("Editor", "Plugins", "Console Manager");
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
	FInputChord OpenLastManagerShortcut = GetMutableDefault<UConsoleManagerSettings>()->OpenLastShortcut;

	bool bAreEqual = OpenManagerShortcut.GetRelationship(OpenLastManagerShortcut) == FInputChord::ERelationshipType::Same;

	if (bAreEqual && OpenManagerShortcut.IsValidChord())
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
		GetMutableDefault<UConsoleManagerSettings>()->OpenShortcut = FInputChord();
		GetMutableDefault<UConsoleManagerSettings>()->OpenLastShortcut = FInputChord();
		auto Section = SettingsModule->GetContainer("Editor")->GetCategory("Plugins")->GetSection("Console Manager");
		Section->Save();

		FNotificationInfo Info(LOCTEXT("WrongShortcuts", "Shortcuts cannot be the same"));

		Info.bFireAndForget = true;
		Info.ExpireDuration = 3.0f;
		Info.FadeOutDuration = 2.0f;
		Info.FadeInDuration = 0.5f;
		Info.bUseSuccessFailIcons = true;

		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Fail);

	}

	OpenManagerShortcut = OpenManagerShortcut.IsValidChord() && !bAreEqual ? OpenManagerShortcut : FInputChord();
	OpenLastManagerShortcut = OpenLastManagerShortcut.IsValidChord() && !bAreEqual ? OpenLastManagerShortcut : FInputChord();

	FConsoleManagerCommands::Get().OpenTab->SetActiveChord(OpenManagerShortcut, EMultipleKeyBindingIndex::Primary);
	FConsoleManagerCommands::Get().OpenTabLast->SetActiveChord(OpenLastManagerShortcut, EMultipleKeyBindingIndex::Primary);


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

	CommandsManager->ShouldLoadAllContainers(true);
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



		ToolBarExtender = MakeShareable(new FExtender);
		ToolBarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FConsoleManagerModule::FillToolbar));

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolBarExtender);


		//UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		//{
		//	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		//	{
		//	
		//		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FConsoleManagerCommands::Get().OpenTab));
		//		Entry.SetCommandList(PluginCommands);

		//		//Section.AddEntry(FToolMenuEntry::InitComboButton(
		//		//	"ConsoleManagerDropdown",
		//		//	FUIAction(
		//		//		FExecuteAction(),
		//		//		FCanExecuteAction(),
		//		//		FIsActionChecked(),
		//		//		FIsActionButtonVisible::CreateStatic(FLevelEditorActionCallbacks::CanShowSourceCodeActions)),
		//		//	FNewToolMenuChoice(),
		//		//	LOCTEXT("CompileCombo_Label", "Compile Options"),
		//		//	LOCTEXT("CompileComboToolTip", "Compile options menu"),
		//		//	FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Recompile"),
		//		//	true
		//		//));

		//	}
		//}
	}
}

void FConsoleManagerModule::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Console Manager");
	{
		// Add a button to edit the current media profile
		ToolbarBuilder.AddToolBarButton(
			FConsoleManagerCommands::Get().OpenTab,
			NAME_None,
			LOCTEXT("ConsoleManager_Label", "ConsoleM"),
			FText::FromString("Opens console manager window"),
			FSlateIcon(FConsoleManagerStyle::GetStyleSetName(), TEXT("ConsoleManager.OpenTab"))
		);

		// Add a simple drop-down menu (no label, no icon for the drop-down button itself) that list the media profile available
		ToolbarBuilder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FConsoleManagerModule::GenerateMenuContent),
			FText::GetEmpty(),
			LOCTEXT("MediaProfileButton_ToolTip", "List of Media Profile available to the user for editing or creation."),
			FSlateIcon(),
			true
		);
	}
	ToolbarBuilder.EndSection();
}

TSharedRef<SWidget> FConsoleManagerModule::GenerateMenuContent()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);

	MenuBuilder.BeginSection("Container", LOCTEXT("NewContainerSection", "New"));
	{
		
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreateMenuLabel", "New Empty Commands Container"),
			LOCTEXT("CreateMenuTooltip", "Create a new Commands Container"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateRaw(this, &FConsoleManagerModule::SpawnNewContainer)
			)
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("Container", LOCTEXT("ContainerSelection", "Containers"));
	{
		TArray<UCommandsContainer*> Containers = LoadAllContainers();

		for (auto Container : Containers)
		{
			const TArray<UCommandsContainer*> Objs = { Container };

			MenuBuilder.AddMenuEntry(
				FText::FromString(Container->GetName()),
				FText::GetEmpty(),
				FSlateIcon(FConsoleManagerStyle::GetStyleSetName(), TEXT("ClassIcon.CommandsContainer")),
				FUIAction(
					FExecuteAction::CreateRaw(this, &FConsoleManagerModule::OpenTabs, Objs)
				)
			);
		}
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FConsoleManagerModule::SpawnNewContainer()
{
	CreateNewContainer();
}

UCommandsContainer* FConsoleManagerModule::CreateNewContainer()
{
	UCommandsContainerFactoryNew* ContainerFactory = DuplicateObject<UCommandsContainerFactoryNew>(GetDefault<UCommandsContainerFactoryNew>(), GetTransientPackage());

	FAssetToolsModule& AssetToolsModule = FAssetToolsModule::GetModule();
	UObject* CreatedAsset = AssetToolsModule.Get().CreateAssetWithDialog(ContainerFactory->GetSupportedClass(), ContainerFactory);

	if (CreatedAsset != nullptr)
	{
		return StaticCast<UCommandsContainer*>(CreatedAsset);
	}
	return nullptr;
}

void FConsoleManagerModule::InitializeCommandsManager(const TArray<UCommandsContainer*>* Containers)
{
	if (Containers != nullptr) 
	{
		CommandsManager->Initialize(*Containers);
	}
	else
	{
		CommandsManager->Initialize();
	}

	// Ask user to create new container if none found
	if (CommandsManager->GetCommandsContainers().Num() == 0)
	{
		FText Title = FText::FromString("New Container");
		EAppReturnType::Type Dec = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("CreateDefaultAssetDialog", "Would you like to create new command container?"), &Title);
		
		if (Dec == EAppReturnType::Type::Yes)
		{
			UCommandsContainer* NewContainer = CreateNewContainer();
			if (NewContainer != nullptr)
			{
				TArray<UCommandsContainer*> Containers = { NewContainer };
				CommandsManager->Initialize(Containers);
			}
		}
	}
}

void FConsoleManagerModule::SpawnOrActivateTab()
{
	if (ActiveTab.IsValid())
	{
		ActiveTab.Pin()->DrawAttention();
		ActiveTab.Pin()->TabActivated();
	}
	else
	{
		if (LastTabManager.IsValid())
		{
			LastTabManager.Pin()->TryInvokeTab(ConsoleManagerTabName);
		}
		else
		{
			FGlobalTabmanager::Get()->TryInvokeTab(ConsoleManagerTabName);
		}
	}
	SaveLastSelectedObjects();
}

void FConsoleManagerModule::SaveLastSelectedObjects()
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
}

TArray<UCommandsContainer*> FConsoleManagerModule::LoadAllContainers()
{
	TArray<FAssetData> Assets;
	UAssetManager::Get().GetAssetRegistry().GetAssetsByClass(UCommandsContainer::StaticClass()->GetFName(), Assets);

	TArray<UCommandsContainer*> Containers;

	UE_LOG(LogTemp, Warning, TEXT("Found %d assets"), Assets.Num());

	for (auto& Asset : Assets)
	{
		//Should check if object is pending kill!
		if (Asset.GetPackage()->IsPendingKill())
		{
			continue;
		}

		UObject* Resolved = Asset.GetAsset();

		if (Resolved)
		{
			UCommandsContainer* Container = StaticCast< UCommandsContainer* >(Resolved);

			Containers.Add(Container);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Finished loading assets!"));

	return Containers;
}

TSharedRef<class SDockTab> FConsoleManagerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//TSharedRef<SDockTab> Tab = SNew(SDockTab)
	//	.TabRole(ETabRole::NomadTab);

	//TWeakObjectPtr<UDeviceProfileManager> DevProf = TWeakObjectPtr<UDeviceProfileManager>(&UDeviceProfileManager::Get());

	//TSharedRef<SWidget> ProfCr = SNew(SDeviceProfileCreateProfilePanel, DevProf);

	//Tab->SetContent(ProfCr);
	//return Tab;

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
			SaveLastSelectedObjects();

			//CommandsManager->SaveToAssets();

			LastTabManager = ActiveTab.Pin()->GetTabManager();
			
			ActiveTab.Reset();
		});

	ConsoleManagerSlate->SetOnTabClosed(ClosedTabDelegate);

	return ConsoleManagerSlate;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FConsoleManagerModule, ConsoleManager)