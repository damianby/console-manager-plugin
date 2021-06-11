// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "CommandsManager.h"

#include "Misc/MessageDialog.h"


#include "Framework/Notifications/NotificationManager.h"

#include "CommandsContainer.h"
#include "Engine/AssetManager.h"

#include "GenericPlatform/GenericPlatformFile.h"

#include "FileHelpers.h"
#include "ISourceControlModule.h"
#include "Misc/FileHelper.h"

#include "Misc/ConfigCacheIni.h"
#include "Misc/OutputDeviceFile.h"

#include "DeviceProfiles/DeviceProfileManager.h"


#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

FCommandsManager::FCommandsManager()
{
	DumpAllCommands();

	LoadConsoleHistory();

	ValidateCommands(ConsoleHistory.Commands);

	SetCurrentCommands(AllCommands);

	// We should refresh our list when new asset with our factory is created
	//FEditorDelegates::OnNewAssetCreated.AddLambda(
	//	[=](UFactory* Factory)
	//	{
	//		// If this is our class then we need to open this container and refresh ui
	//		if (Factory->GetSupportedClass() == UCommandsContainer::StaticClass() && bShouldLoadNew)
	//		{
	//			
	//			UE_LOG(LogTemp, Warning, TEXT("New asset LOADING!"));
	//			TArray<UCommandsContainer*> Containers = LoadAllContainers();

	//			for (int i = 0; i < Containers.Num(); i++) 
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("Container found: %s"), *Containers[i]->GetName());
	//			}

	//			//Initialize_Internal(Containers);
	//			/*for (auto& GroupInContainer : Container->Groups)
	//			{
	//				GroupToContainerMap.Add(GroupInContainer.Id, Container);
	//			}*/
	//			
	//		}

	//		UE_LOG(LogTemp, Warning, TEXT("New Asset created! %s"), *Factory->GetClass()->GetName());
	//	});
	// No need to look for deleted as we should check everytime if object is valid

	FCoreUObjectDelegates::OnPackageReloaded.AddLambda(
		[this](EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
		{
			if (Phase == EPackageReloadPhase::OnPackageFixup)
			{
				UE_LOG(LogTemp, Warning, TEXT("Package %s reloaded"), *Event->GetNewPackage()->GetName());

				UCommandsContainer* RepointedContainer = nullptr;
				int IndexToInsert = 0;

				for (int i = 0; i < CommandsContainers.Num(); i++)
				{
					UCommandsContainer* Container = CommandsContainers[i];

					if (Event->GetRepointedObject(Container, RepointedContainer))
					{
						IndexToInsert = i;
						UE_LOG(LogTemp, Warning, TEXT("Container %s must be repointed"), *Container->GetName());

					}	
				}

				if (RepointedContainer != nullptr)
				{
					CommandsContainers.Insert(RepointedContainer, IndexToInsert);
				}
				// No need to refresh after because old package will be destroyed triggering event to refresh all
				
			}
		});
	
	Snapshot.Id = GetNewIdForGroup();
	Snapshot.bIsEditable = false;

	EPluginLoadedFrom LoadedFrom = IPluginManager::Get().FindPlugin("ConsoleManager")->GetLoadedFrom();

	switch (LoadedFrom)
	{
	case EPluginLoadedFrom::Engine:

		break;

	case EPluginLoadedFrom::Project:

		break;
	}

	// Find or create default local container (outside SCC)

	//const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetContentDir() / TEXT("");
	////FString PerforceIgnoreList = TEXT(".p4ignore.txt") LINE_TERMINATOR TEXT("LocalContainer") + FPackageName::GetAssetPackageExtension();

	////FString IgnorePath = CommandsPath / TEXT(".p4ignore");
	////FFileHelper::SaveStringToFile(PerforceIgnoreList, *IgnorePath);


	//const FString MountedPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetMountedAssetPath();

	//FString LocalContainerPackageName = MountedPath / TEXT("LocalContainer");

	//UPackage* Package = CreatePackage(NULL, *LocalContainerPackageName);
	//UCommandsContainer* NewContainer = NewObject<UCommandsContainer>(Package, UCommandsContainer::StaticClass(), *FString("LocalContainer"), 
	//	EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	//
	//FAssetRegistryModule::AssetCreated(NewContainer);
	//
	//

	//FString FilePath = FString::Printf(TEXT("%s%s%s"), *CommandsPath, *FString("LocalContainer"), *FPackageName::GetAssetPackageExtension());

	//bool bSuccess = UPackage::SavePackage(Package, NewContainer, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);

	//
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *CommandsPath);


	//FConfigCacheIni::G


	TArray<UObject*> Profiles = UDeviceProfileManager::Get().Profiles;

	for (auto& Profile : Profiles)
	{
		UE_LOG(LogTemp, Warning, TEXT("Profile %s"), *Profile->GetName());
	}
	

	if (GConfig->IsReadyForUse())
	{
		UE_LOG(LogTemp, Warning, TEXT("IS READY TO USE GCONFIG"));

		TArray<FString> Arr;

		GConfig->GenerateKeyArray(Arr);

		

		TArray<FConfigFile> CArr;

		for (auto El : Arr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Config %s"), *El);
		}

		/*GConfig->GenerateValueArray(CArr);*/
		//CArr[0].OverrideFileFromCommandline
		//GConfig->Dump
		//const FString Path = FPaths::Combine("F:", "Ini.ini");
	

		TArray<FString> LocalCommands;

		FStringOutputDevice StringOutDevice;
		StringOutDevice.SetAutoEmitLineTerminator(true);

		GConfig->Dump(StringOutDevice);
		//ConsoleCommandLibrary_DumpLibrary(GEditor->GetEditorWorldContext().World(), *GEngine, FString(TEXT("")) + TEXT("*"), StringOutDevice);

		StringOutDevice.ParseIntoArrayLines(LocalCommands);

		UE_LOG(LogTemp, Warning, TEXT("Local Commands lines: %d"), LocalCommands.Num());


		const FString Path = FPaths::GeneratedConfigDir() + TEXT("Linesall.txt");
		FileHelper::DumpAllCommands(Path, LocalCommands);

		

		// Create a file writer to dump the info to
		//FOutputDevice* OutputOverride = nullptr;
		//FString OutputFilename;
		//TUniquePtr<FOutputDeviceFile> OutputBuffer;
		////if (FParse::Value(*Params, TEXT("dumptofile="), OutputFilename))
		////{
		//	OutputBuffer = MakeUnique<FOutputDeviceFile>(*Path, true);
		//	OutputBuffer->SetSuppressEventTag(true);
		//	OutputOverride = OutputBuffer.Get();

			
		//}



		

			//FCommandLine::Get()
	}



	FConsoleCommandDelegate Delegate;

	Delegate.BindRaw(this, &FCommandsManager::VariableChanged);

	Handle = IConsoleManager::Get().RegisterConsoleVariableSink_Handle(Delegate);
}

FCommandsManager::~FCommandsManager()
{
	IConsoleManager::Get().UnregisterConsoleVariableSink_Handle(Handle);
}

void FCommandsManager::VariableChanged()
{
	if (!bSinkBlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sink called!"));
		UpdateHistory();

		OnCommandsRefresh.ExecuteIfBound();
	}
}

void FCommandsManager::Initialize_Internal(const TArray<UCommandsContainer*>& Containers)
{
	CommandsContainers = Containers;

	if (CommandsContainers.Num() == 0)
	{
		CommandsContainers = LoadAllContainers();
	}

	GroupToContainerMap.Empty();

	for (auto Container : CommandsContainers)
	{
		if (Container->IsPendingKillOrUnreachable() || !Container->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping container, its either invalid, pending kill or unreachable"));
			continue;
		}
		Container->OnDestroyCalled.BindRaw(this, &FCommandsManager::ContainerBeingDestroyed);
		Container->OnRenamed.BindRaw(this, &FCommandsManager::ContainerRenamed);

		for (auto& GroupInContainer : Container->Groups)
		{
			GroupToContainerMap.Add(GroupInContainer.Id, Container);
		}
	}

	OnDataRefreshed.ExecuteIfBound();
}

void FCommandsManager::Initialize()
{
	TArray<UCommandsContainer*> Containers;
	Initialize_Internal(Containers);
}

void FCommandsManager::Initialize(const TArray<UCommandsContainer*>& Containers)
{
	Initialize_Internal(Containers);
}

const TArray<TPair<FString, FGuid>> FCommandsManager::GetGroupList()
{
	TArray<TPair<FString, FGuid>> Groups;

	for (int i = 0; i < CommandsContainers.Num(); i++)
	{
		const auto& Container = CommandsContainers[i];
		for (int j = 0; j < Container->Groups.Num(); j++)
		{
			Groups.Add(TPair<FString, FGuid>(Container->Groups[j].Name, Container->Groups[j].Id));
		}
	}

	return Groups;
}

bool FCommandsManager::SetActiveGroup(FGuid Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		if (Container->IsValidLowLevel())
		{
			FCommandGroup* FoundGroup = Container->Groups.FindByKey<FGuid>(Id);

			SetCurrentCommands(*FoundGroup);
			
			CurrentContainer = Container;
			return true;
		}
	}

	return false;
}

void FCommandsManager::ReorderCommandInCurrentGroup(int32 CurrentId, int32 NewId)
{
	check(CurrentGroup);

	if (CurrentId == NewId)
	{
		return;
	}

	TArray<FConsoleCommand>& Commands = CurrentGroup->Commands;
	if (NewId > CurrentId)
	{
		//Initialize after insert to avoid crash with relocalization
		Commands.InsertZeroed_GetRef(NewId) = Commands[CurrentId];
		Commands.RemoveAt(CurrentId);
	}
	else
	{
		FConsoleCommand Cpy = Commands[CurrentId];
		Commands.RemoveAt(CurrentId, 1, false);
		Commands.Insert(Cpy, NewId);
	}

	ContainerChanged();
}

void FCommandsManager::DuplicateCommand(int32 Id)
{
	FConsoleCommand Cpy = CurrentGroup->Commands[Id];
	CurrentGroup->Commands.Insert(Cpy, Id + 1);

	ContainerChanged();
}

void FCommandsManager::RemoveCommands(TArray<int32> Ids)
{
	Ids.Sort();

	TArray<FConsoleCommand>& Commands = CurrentGroup->Commands;

	//Start removing from end of array disallowing shrinking
	for (int i = Ids.Num() - 1; i >= 0; i--)
	{
		Commands.RemoveAt(Ids[i], 1, false);
	}

	ContainerChanged();
}

void FCommandsManager::SetNoteCommand(int32 Id, const FString& NewNote)
{
	TArray<FConsoleCommand>& Commands = CurrentGroup->Commands;

	if (Commands.IsValidIndex(Id))
	{
		Commands[Id].SetNote(NewNote);
	}
}

bool FCommandsManager::SetActiveHistory()
{
	if (CurrentGroup != &ConsoleHistory)
	{
		SetCurrentCommands(ConsoleHistory);
		return true;
	}
	return false;
}

bool FCommandsManager::SetActiveAllCommands()
{
	if (CurrentGroup != &AllCommands)
	{
		SetCurrentCommands(AllCommands);
		return true;
	}
	return false;
}

bool FCommandsManager::SetActiveSnapshot()
{
	if (CurrentGroup != &Snapshot)
	{
		SetCurrentCommands(Snapshot);
		return true;
	}
	return false;
}

void FCommandsManager::AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	if (Group)
	{
		if (Group->bIsEditable)
		{
			for (const auto& Command : Commands)
			{
				// Make double copy to avoid adding elements that are from this container
				FConsoleCommand& NewCommand = Group->Commands.Add_GetRef(FConsoleCommand(*Command.Get()));

				if (NewCommand.GetValue().IsEmpty())
				{
					NewCommand.SetValue(Command->GetCurrentValue());
				}
			}
		}

		if (Group->Id == CurrentGroup->Id)
		{
			OnCommandsRefresh.ExecuteIfBound();
		}
	}
}

void FCommandsManager::AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	AddCommandsToGroup(CurrentGroup, Commands);
	ContainerChanged();
}

void FCommandsManager::AddCommandsToGroup(FGuid Id, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	FCommandGroup* Group = GetGroup(Id);

	AddCommandsToGroup(Group, Commands);
	ContainerChanged();
}

void FCommandsManager::UpdateCurrentEngineValue(const FConsoleCommand& Command)
{
	bool SuccessExecuting = Execute_Internal(Command);
}

void FCommandsManager::ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand)
{
	check(CurrentGroup->Commands.IsValidIndex(Id));

	CurrentGroup->Commands[Id] = NewCommand;
	
	ContainerChanged();
}

const FCommandGroup* FCommandsManager::GetGroupById(const FGuid& Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		FCommandGroup* FoundGroup = Container->Groups.FindByKey<FGuid>(Id);

		if (FoundGroup)
		{
			return FoundGroup;
		}
	}

	return nullptr;
}

const FConsoleCommand& FCommandsManager::GetConsoleCommand(int Id)
{
	return CurrentGroup->Commands[Id];
}

bool FCommandsManager::ExecuteCommand(const FConsoleCommand& Command)
{
	return Execute_Internal(Command);
}

bool FCommandsManager::ExecuteCommand(FConsoleCommand& Command)
{
	bool SuccessExecuting = Execute_Internal(Command);

	Command.SetIsValid(SuccessExecuting);

	const FString& ExecCommand = Command.GetExec();

	FString ExecStateDisplay = SuccessExecuting ? FString("executed succesfully!") : FString("failed!");

	DisplayNotification(FText::FromString(FString::Printf(TEXT("%s %s"), *ExecCommand, *ExecStateDisplay)),
		SuccessExecuting ? SNotificationItem::ECompletionState::CS_Success : SNotificationItem::ECompletionState::CS_Fail);

	return SuccessExecuting;
}

void FCommandsManager::ExecuteMultipleCommands(TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	TArray<FString> Errors;
	for (const auto& Command : Commands)
	{
		if (!Execute_Internal(Command.ToSharedRef().Get()))
		{
			Errors.Add(Command->GetExec());
		}
	}

	if (Errors.Num() > 0)
	{
		FString Result;

		for (const FString& Error : Errors)
		{
			Result.Append(Error);
			Result.Append(LINE_TERMINATOR);
		}

		DisplayNotification(FText::FromString(FString::Printf(TEXT("%d/%d commands executed succesfully!\nCommands failed:\n%s"), Commands.Num() - Errors.Num(), Commands.Num(), *Result)),
			SNotificationItem::ECompletionState::CS_None
		);
	}
	else
	{
		DisplayNotification(FText::FromString(FString::Printf(TEXT("%d commands executed succesfully!"), Commands.Num())), SNotificationItem::ECompletionState::CS_Success);
	}
}

void FCommandsManager::ExecuteGroup(const FGuid& Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (!FoundContainer)
	{
		return;
	}

	UCommandsContainer* Container = *FoundContainer;
	int32 GroupIndex = Container->Groups.IndexOfByKey<FGuid>(Id);

	if (GroupIndex == INDEX_NONE)
	{
		return;
	}

	FCommandGroup& FoundGroup = Container->Groups[GroupIndex];

	TArray<FString> Errors;
	for (const auto& Command : FoundGroup.Commands)
	{
		if (!Execute_Internal(Command))
		{
			Errors.Add(Command.GetExec());
		}
	}

	if (Errors.Num() > 0)
	{
		FString Result;

		for (const FString& Error : Errors)
		{
			Result.Append(Error);
			Result.Append(LINE_TERMINATOR);

		}

		DisplayNotification(FText::FromString(FString::Printf(TEXT("%s\n%d/%d commands executed succesfully!\nCommands failed:\n%s"), *FoundGroup.Name,
			FoundGroup.Commands.Num() - Errors.Num(), FoundGroup.Commands.Num(), *Result)),
			SNotificationItem::ECompletionState::CS_None
			);
	}
	else
	{
		DisplayNotification(FText::FromString(FString::Printf(TEXT("%s succesfully executed"), *FoundGroup.Name)),
			SNotificationItem::ECompletionState::CS_Success
			);
	}
}

void FCommandsManager::RemoveGroup(FGuid Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;
		int32 GroupIndex = Container->Groups.IndexOfByKey<FGuid>(Id);

		if (GroupIndex > INDEX_NONE)
		{
			FCommandGroup& FoundGroup = Container->Groups[GroupIndex];

			if (CurrentGroup->Id == FoundGroup.Id)
			{
				SetCurrentCommands(AllCommands);
			}

			Container->Groups.RemoveAt(GroupIndex, 1, false);
		}
	}

	GroupToContainerMap.Remove(Id);
}


void FCommandsManager::CreateNewGroup(const FString& Name, UCommandsContainer* Container)
{
	if (Container->IsValidLowLevel() && Container->GetGroupByName(Name) == nullptr)
	{
		FCommandGroup& NewGroup = AddNewGroup_Internal(Name, Container);
		ContainerChanged(Container);
		OnGroupsRefresh.ExecuteIfBound();

		DisplayNotification(LOCTEXT("AddNewGroup_Notification", "Group created!"), SNotificationItem::ECompletionState::CS_Success);
	}
	else
	{
		DisplayNotification(LOCTEXT("AddNewGroup_Error_Notification", "Group with given name exists in selected container!"), SNotificationItem::ECompletionState::CS_Fail);
	}
}

void FCommandsManager::CreateNewGroup(const FString& Name, UCommandsContainer* Container, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	if (Container->IsValidLowLevel() && Container->GetGroupByName(Name) == nullptr)
	{
		FCommandGroup& NewGroup = AddNewGroup_Internal(Name, Container);
		AddCommandsToGroup(&NewGroup, Commands);
		ContainerChanged(Container);
		OnGroupsRefresh.ExecuteIfBound();

		DisplayNotification(LOCTEXT("AddNewGroup_Notification", "Group created!"), SNotificationItem::ECompletionState::CS_Success);
	}
	else
	{
		DisplayNotification(LOCTEXT("AddNewGroup_Error_Notification", "Group with given name exists in selected container!"), SNotificationItem::ECompletionState::CS_Fail);
	}
}

void FCommandsManager::CreateSnapshotCVars()
{
	Snapshot.Commands.Empty();
	for (auto& Command : AllCommands.Commands)
	{
		if (Command.GetObjType() == EConsoleCommandType::CVar)
		{
			//Update current value
			Command.Refresh();
			FConsoleCommand& CopiedCommand = Snapshot.Commands.Add_GetRef(Command);
			CopiedCommand.SetValue(Command.GetCurrentValue());
		}
	}
	if (CurrentGroup->Id == Snapshot.Id)
	{
		RebuildSharedArray();
		OnCommandsRefresh.ExecuteIfBound();
	}
}

void FCommandsManager::RevertSnapshotCVars()
{
	//Block sink to avoid calling after every variable change
	bSinkBlocked = true;

	for(auto& Command : Snapshot.Commands)
	{
		if (Command.GetObjType() == EConsoleCommandType::CVar)
		{
			Execute_Internal(Command, false);
		}
	}
	bSinkBlocked = false;

	OnCommandsRefresh.ExecuteIfBound();
}

FCommandGroup& FCommandsManager::AddNewGroup_Internal(const FString& Name, UCommandsContainer* Container, EGroupType Type)
{
	FCommandGroup& NewGroup = Container->Groups.AddDefaulted_GetRef();
	NewGroup.Type = Type;
	NewGroup.Name = Name;
	NewGroup.Id = GetNewIdForGroup();

	GroupToContainerMap.Add(NewGroup.Id, Container);

	return NewGroup;
}

bool FCommandsManager::RenameGroup(FGuid Id, const FString& NewName)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		FCommandGroup* FoundGroup = Container->Groups.FindByKey<FGuid>(Id);

		FoundGroup->Name = NewName;
		ContainerChanged(Container);

		OnGroupsRefresh.ExecuteIfBound();

		return true;
	}

	return false;
}

void FCommandsManager::DuplicateGroup(FGuid Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		int32 GroupIndex = Container->Groups.IndexOfByKey<FGuid>(Id);

		if (GroupIndex > INDEX_NONE)
		{
			FCommandGroup NewGroup = Container->Groups[GroupIndex];
			NewGroup.Name = NewGroup.Name + "_Dup";
			NewGroup.Id = GetNewIdForGroup();


			Container->Groups.Insert(NewGroup, GroupIndex + 1);

			GroupToContainerMap.Add(NewGroup.Id, Container);
			ContainerChanged(Container);
			OnGroupsRefresh.ExecuteIfBound();
		}
	}
}

FGuid FCommandsManager::GetNewIdForGroup()
{
	return FGuid::NewGuid();
}

void FCommandsManager::UpdateHistory()
{
	TArray<FString> Out;
	IConsoleManager::Get().GetConsoleHistory(*FString(), Out);
	
	//// limit size to avoid a ever growing file
	//while (ConsoleHistory.Commands.Num() > HistoryBufferSize)
	//{
	//	ConsoleHistory.Commands.RemoveAt(0);
	//}
	if (Out.Num() > 0) 
	{
		FString ExecCommand = Out.Last();

		if (ExecCommand.Equals(ConsoleHistory.Commands.Last().ExecCommand, ESearchCase::IgnoreCase))
		{
			//ConsoleHistory.Commands.RemoveAll([&ExecCommand](FConsoleCommand& Command) { return Command.GetExec().Equals(ExecCommand, ESearchCase::IgnoreCase); });
			ConsoleHistory.Commands.Add(FConsoleCommand(ExecCommand));


			if (CurrentGroup->Type == EGroupType::History)
			{
				RebuildSharedArray();
			}
		}
	}
}

 TArray<UCommandsContainer*> FCommandsManager::LoadAllContainers()
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

void FCommandsManager::SaveToAssets()
{
	if (ISourceControlModule::Get().IsEnabled())
	{
		TArray<UPackage*> PackageList;
		for (auto Container : CommandsContainers)
		{	
			PackageList.Add(Container->GetPackage());
		}

		FEditorFileUtils::PromptForCheckoutAndSave(PackageList, true, false);
	}
	else
	{
		TArray<UObject*> NotSavedAssets;
		// If disabled just go through and save everything
		for (auto& Container : CommandsContainers)
		{
			UPackage* Package = Container->GetPackage();

			UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());

			// Construct a filename from long package name.
			FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());

			if (!IPlatformFile::GetPlatformPhysical().IsReadOnly(*PackageFileName))
			{
				TArray<UPackage*> PackagesToSave = { Package };
				FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, true);
				//FSavePackageResultStruct OutStruct = Package->Save(Package, Container, EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *PackageFileName);
				//UE_LOG(LogTemp, Warning, TEXT("Result of save: %d | %lld"), OutStruct.Result, OutStruct.TotalFileSize);
			}
			else
			{
				NotSavedAssets.Add(Container);
			}
		}

		if (NotSavedAssets.Num() > 0)
		{	
			TArray<UObject*> OutList;
			FEditorFileUtils::SaveAssetsAs(NotSavedAssets, OutList);
		}
	}
	//FUnrealEdMisc::
}

void FCommandsManager::Refresh()
{
	if (bLoadAllContainers) 
	{
		TArray<UCommandsContainer*> NewContainers = LoadAllContainers();
		Initialize_Internal(NewContainers);
	}
	else
	{
		OnDataRefreshed.ExecuteIfBound();
	}
}

void FCommandsManager::ShouldLoadAllContainers(bool Val)
{
	bLoadAllContainers = Val;
}

void FCommandsManager::LoadConsoleHistory()
{
	ConsoleHistory.Name = "History";
	ConsoleHistory.Id = FGuid::NewGuid();
	ConsoleHistory.Type = EGroupType::History;
	ConsoleHistory.bIsEditable = false;

	FileHelper::LoadConsoleHistory(ConsoleHistory.Commands);

	// limit size to avoid a ever growing file
	while (ConsoleHistory.Commands.Num() > HistoryBufferSize)
	{
		ConsoleHistory.Commands.RemoveAt(0);
	}
}

void FCommandsManager::RebuildSharedArray()
{
	int CommandsNum = CurrentGroup->Commands.Num();

	CurrentCommandsShared.Reset(CommandsNum);

	UE_LOG(LogTemp, Warning, TEXT("Rebuild shared array!"));

	for (int i = 0; i < CommandsNum; i++)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Shared before : %s : %s : %s : %s"), *CurrentGroup->Commands[i].GetName(), *CurrentGroup->Commands[i].GetValue(), *CurrentGroup->Commands[i].GetExec(), *CurrentGroup->Commands[i].GetSetBy());

		CurrentCommandsShared.Add(MakeShareable(&CurrentGroup->Commands[i], FDeleterNot()));
		//UE_LOG(LogTemp, Warning, TEXT("Shared element: %s : %s : %s : %s"), *CurrentCommandsShared.Last()->GetName(), *CurrentCommandsShared.Last()->GetValue(), *CurrentCommandsShared.Last()->GetExec(), *CurrentCommandsShared.Last()->GetSetBy());
	}
}

void FCommandsManager::ContainerChanged(UCommandsContainer* Container)
{
	if (!Container)
	{
		Container = CurrentContainer;
		RebuildSharedArray();
		Container->GetPackage()->MarkPackageDirty();
	}
	else
	{
		UCommandsContainer** ContainerFound_Ptr = GroupToContainerMap.Find(CurrentGroup->Id);
		if (ContainerFound_Ptr)
		{
			UCommandsContainer* ContainerFound = *ContainerFound_Ptr;

			if (ContainerFound == Container)
			{
				RebuildSharedArray();
			}
		}

		if (Container->IsValidLowLevel())
		{
			Container->GetPackage()->MarkPackageDirty();
		}
	}
}

//It will not add wrong command to history
bool FCommandsManager::Execute_Internal(const FConsoleCommand& Command, bool UpdateHistory)
{
	//IModularFeatures::Get().RegisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), CmdExec.Get());

	//IConsoleCommandExecutor& CmdExec = IModularFeatures::Get().GetModularFeature<IConsoleCommandExecutor>("ConsoleCommandExecutor");

	//CmdExec.Exec()
	const FString& ExecCommand = Command.GetExec();

	bool SuccessExecuting = GEngine->Exec(GEngine->GetWorldContexts().Last().World(), *ExecCommand, *GLog);

	if (SuccessExecuting && UpdateHistory)
	{
		IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *ExecCommand);
	}

	return SuccessExecuting;

	//HERE Exec from FConsoleCommandExecutor
	/*
		IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), Input);

	bool bWasHandled = false;
	UWorld* World = nullptr;
	UWorld* OldWorld = nullptr;

	// The play world needs to handle these commands if it exists
	if (GIsEditor && GEditor->PlayWorld && !GIsPlayInEditorWorld)
	{
		World = GEditor->PlayWorld;
		OldWorld = SetPlayInEditorWorld(GEditor->PlayWorld);
	}

	ULocalPlayer* Player = GEngine->GetDebugLocalPlayer();
	if (Player)
	{
		UWorld* PlayerWorld = Player->GetWorld();
		if (!World)
		{
			World = PlayerWorld;
		}
		bWasHandled = Player->Exec(PlayerWorld, Input, *GLog);
	}

	if (!World)
	{
		World = GEditor->GetEditorWorldContext().World();
	}
	if (World)
	{
		if (!bWasHandled)
		{
			AGameModeBase* const GameMode = World->GetAuthGameMode();
			AGameStateBase* const GameState = World->GetGameState();
			if (GameMode && GameMode->ProcessConsoleExec(Input, *GLog, nullptr))
			{
				bWasHandled = true;
			}
			else if (GameState && GameState->ProcessConsoleExec(Input, *GLog, nullptr))
			{
				bWasHandled = true;
			}
		}

		if (!bWasHandled && !Player)
		{
			if (GIsEditor)
			{
				bWasHandled = GEditor->Exec(World, Input, *GLog);
			}
			else
			{
				bWasHandled = GEngine->Exec(World, Input, *GLog);
			}
		}
	}

	// Restore the old world of there was one
	if (OldWorld)
	{
		RestoreEditorWorld(OldWorld);
	}

	return bWasHandled;
	
	*/
}

void FCommandsManager::ValidateCommands(TArray<FConsoleCommand>& Commands)
{
	for (auto& Command : Commands)
	{
		if (!AllCommands.Commands.FindByKey<FString>(Command.GetName()))
		{
			Command.SetIsValid(false);
		}
	}
}

void FCommandsManager::SetCurrentCommands(FCommandGroup& Group)
{
	CurrentGroup = &Group;
	RebuildSharedArray();
}

void FCommandsManager::ContainerBeingDestroyed(UCommandsContainer* Container)
{
	if (Container == CurrentContainer)
	{
		SetActiveAllCommands();
	}

	if (Container)
	{
		bool bContainerRemoved = static_cast<bool>(CommandsContainers.RemoveSingle(Container));

		if (bContainerRemoved)
		{
			// We shouldnt call Initialize_Internal from OnDestroy event when there are
			// no containers left because it might trigger garbage collector error
			if (CommandsContainers.Num() > 0)
			{
				Initialize_Internal(CommandsContainers);
			}
			UE_LOG(LogTemp, Warning, TEXT("Container %s is dying! Removed: %d"), *Container->GetName(), bContainerRemoved);
		}
	}	
}

void FCommandsManager::ContainerRenamed(UCommandsContainer* Container)
{
	OnGroupsRefresh.ExecuteIfBound();
}

void FCommandsManager::DumpAllCommands()
{
	TArray<FString> LocalCommands;

	FStringOutputDevice StringOutDevice;
	StringOutDevice.SetAutoEmitLineTerminator(true);
	ConsoleCommandLibrary_DumpLibrary(GEditor->GetEditorWorldContext().World(), *GEngine, FString(TEXT("")) + TEXT("*"), StringOutDevice);
	StringOutDevice.ParseIntoArrayLines(LocalCommands);

	//FConsoleObjectVisitor Visitor;
	//Visitor.BindLambda([&LocalCommands](const TCHAR* Name, IConsoleObject* Obj) {

	//	LocalCommands.Add(Name);
	//});

	//IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(Visitor, TEXT("*"));

	//LocalCommands.Sort(TLess<FString>());

	AllCommands.Commands.Empty();
	AllCommands.Name = "All Commands";
	AllCommands.Id = FGuid::NewGuid();
	AllCommands.bIsEditable = false;
	AllCommands.Type = EGroupType::AllCommands;

	for (auto& Command : LocalCommands)
	{
		AllCommands.Commands.Add(FConsoleCommand(Command));
	}

	const FString Path = FPaths::GeneratedConfigDir() + TEXT("Console.txt");
	
	FileHelper::DumpAllCommands(Path, LocalCommands);
}

void FCommandsManager::DisplayNotification(FText Text, SNotificationItem::ECompletionState State)
{
	FNotificationInfo Info(Text);

	Info.bFireAndForget = true;
	Info.ExpireDuration = 3.0f;
	Info.FadeOutDuration = 2.0f;
	Info.FadeInDuration = 0.5f;
	Info.bUseSuccessFailIcons = true;

	if (NotificationItem.IsValid())
	{
		NotificationItem->Fadeout();
	}

	NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(State);
}

FCommandGroup* FCommandsManager::GetGroup(FGuid Id)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		FCommandGroup* FoundGroup = Container->Groups.FindByKey<FGuid>(Id);

		if (FoundGroup)
		{
			return FoundGroup;
		}
	}

	return nullptr;
}


#undef LOCTEXT_NAMESPACE
