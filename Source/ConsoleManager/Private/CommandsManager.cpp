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


#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

FCommandsManager::FCommandsManager()
{
	DumpAllCommands();

	LoadConsoleHistory();

	ValidateCommands(ConsoleHistory.Commands);

	FileHelper::PrintGroups_Debug(CommandGroups);

	SetCurrentCommands(ConsoleHistory);

	// We should refresh our list when new asset with our factory is created
	FEditorDelegates::OnNewAssetCreated.AddLambda(
		[=](UFactory* Factory)
		{
			
			UE_LOG(LogTemp, Warning, TEXT("New Asset created! %s"), *Factory->GetClass()->GetName());
		});
	// No need to look for deleted as we should check everytime if object is valid

	FCoreUObjectDelegates::OnPackageReloaded.AddLambda(
		[this](EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
		{
			if (Phase == EPackageReloadPhase::OnPackageFixup)
			{
				UE_LOG(LogTemp, Warning, TEXT("Package %s reloaded"), *Event->GetNewPackage()->GetName());


				for (auto& Container : CommandsContainers)
				{
					Event->RepointObject<UCommandsContainer>(Container);
				}
				OnDataRefreshed.ExecuteIfBound();
			}
		});
	
	Snapshot.Id = GetNewIdForGroup(Snapshot);

	EPluginLoadedFrom LoadedFrom = IPluginManager::Get().FindPlugin("ConsoleManager")->GetLoadedFrom();

	switch (LoadedFrom)
	{
	case EPluginLoadedFrom::Engine:

		break;

	case EPluginLoadedFrom::Project:

		break;
	}

	// Find or create default local container (outside SCC)

	const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetContentDir() / TEXT("");
	//FString PerforceIgnoreList = TEXT(".p4ignore.txt") LINE_TERMINATOR TEXT("LocalContainer") + FPackageName::GetAssetPackageExtension();

	//FString IgnorePath = CommandsPath / TEXT(".p4ignore");
	//FFileHelper::SaveStringToFile(PerforceIgnoreList, *IgnorePath);


	const FString MountedPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetMountedAssetPath();

	FString LocalContainerPackageName = MountedPath / TEXT("LocalContainer");

	UPackage* Package = CreatePackage(NULL, *LocalContainerPackageName);
	UCommandsContainer* NewContainer = NewObject<UCommandsContainer>(Package, UCommandsContainer::StaticClass(), *FString("LocalContainer"), 
		EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	
	FAssetRegistryModule::AssetCreated(NewContainer);
	
	

	FString FilePath = FString::Printf(TEXT("%s%s%s"), *CommandsPath, *FString("LocalContainer"), *FPackageName::GetAssetPackageExtension());

	bool bSuccess = UPackage::SavePackage(Package, NewContainer, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FilePath);

	
	UE_LOG(LogTemp, Warning, TEXT("%s"), *CommandsPath);

}

void FCommandsManager::Initialize_Internal(TArray<UCommandsContainer*> Containers)
{
	CommandsContainers = Containers;

	if (CommandsContainers.Num() == 0)
	{
		CommandsContainers = LoadAllContainers();
	}

	for (auto& Container : CommandsContainers)
	{
		if (!Container->IsRooted())
		{
			Container->AddToRoot();
		}

		for (auto& GroupInContainer : Container->Groups)
		{
			GroupToContainerMap.Add(GroupInContainer.Id, Container);
		}
	}
}

void FCommandsManager::Initialize()
{
	Initialize_Internal(LoadAllContainers());
}

void FCommandsManager::Initialize(TArray<UCommandsContainer*> Containers)
{
	Initialize_Internal(Containers);
}

void FCommandsManager::Refresh()
{
	//const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources") / TEXT("Commands.txt");
	

	
	//FPaths::EngineContentDir().PathAppend()


	//FileHelper::ReadCommandFile(CommandsPath, CommandGroups);
	

	//for (auto& Group : CommandGroups)
	//{
	//	ValidateCommands(Group.Commands);
	//}

	//FCommandGroup* CurrentGroupByName = CommandGroups.FindByKey<FString>(CurrentGroupId);
	//if (CurrentGroupByName)
	//{
	//	SetCurrentCommands(*CurrentGroupByName);
	//}
	//else
	//{
	//	SetCurrentCommands(ConsoleHistory);
	//}
}

const TArray<TSharedPtr<FConsoleCommand>>& FCommandsManager::GetCurrentSharedCommands()
{
	return CurrentCommandsShared;
}

const TArray<FConsoleCommand>& FCommandsManager::GetCurrentCommands()
{
	return CurrentGroup->Commands;
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

	RebuildSharedArray();
}

void FCommandsManager::DuplicateCommand(int32 Id)
{
	FConsoleCommand Cpy = CurrentGroup->Commands[Id];
	CurrentGroup->Commands.Insert(Cpy, Id + 1);

	RebuildSharedArray();
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

	RebuildSharedArray();
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
				FConsoleCommand& NewCommand = Group->Commands.Add_GetRef(*Command.Get());

				if (NewCommand.GetValue().IsEmpty())
				{
					NewCommand.SetValue(Command->GetCurrentValue());
				}
			}
			RebuildSharedArray();
		}
	}
}

void FCommandsManager::AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	AddCommandsToGroup(CurrentGroup, Commands);
}

void FCommandsManager::AddCommandsToGroup(FGuid Id, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	FCommandGroup* Group = GetGroup(Id);

	AddCommandsToGroup(Group, Commands);
}

void FCommandsManager::UpdateCurrentEngineValue(const FConsoleCommand& Command)
{
	bool SuccessExecuting = Execute(Command);
}

void FCommandsManager::ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand)
{
	check(CurrentGroup->Commands.IsValidIndex(Id));

	CurrentGroup->Commands[Id] = NewCommand;
	
	RebuildSharedArray();
}

const FCommandGroup* FCommandsManager::GetAllCommands()
{
	return &AllCommands;
}

const FCommandGroup* FCommandsManager::GetHistory()
{
	return &ConsoleHistory;
}

const FCommandGroup* FCommandsManager::GetSnapshot()
{
	return &Snapshot;
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
	return Execute(Command);
}

bool FCommandsManager::ExecuteCommand(FConsoleCommand& Command)
{
	bool SuccessExecuting = Execute(Command);

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
		if (!Execute(Command.ToSharedRef().Get()))
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
		if (!Execute(Command))
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

//void FCommandsManager::RemoveGroup(int Id)
//{
//	check(CommandGroups.IsValidIndex(Id));
//
//	if (CurrentGroup == &CommandGroups[Id])
//	{
//		SetCurrentCommands(AllCommands);
//	}
//
//	CommandGroups.RemoveAt(Id, 1, false);
//
//}

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
}


void FCommandsManager::CreateNewGroup(const FString& Name, UCommandsContainer* Container)
{
	if (Container->IsValidLowLevel())
	{
		FCommandGroup& NewGroup = AddNewGroup_Internal(Name, Container);
		OnDataRefreshed.ExecuteIfBound();
	}
}

void FCommandsManager::CreateNewGroup(const FString& Name, UCommandsContainer* Container, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	if (Container->IsValidLowLevel())
	{
		FCommandGroup& NewGroup = AddNewGroup_Internal(Name, Container);
		AddCommandsToGroup(&NewGroup, Commands);
		OnDataRefreshed.ExecuteIfBound();
	}
}

void FCommandsManager::CreateSnapshotCVars(const FString& Name, UCommandsContainer* Container)
{
	if (Container->IsValidLowLevel())
	{
		FCommandGroup& NewGroup = AddNewGroup_Internal(Name, Container);

		for (const auto& Command : AllCommands.Commands)
		{
			if (Command.GetObjType() == EConsoleCommandType::CVar)
			{
				FConsoleCommand& CopiedCommand = NewGroup.Commands.Add_GetRef(Command);
				CopiedCommand.SetValue(Command.GetCurrentValue());
			}
		}
	}
}

FCommandGroup& FCommandsManager::AddNewGroup_Internal(const FString& Name, UCommandsContainer* Container, EGroupType Type)
{
	FCommandGroup& NewGroup = Container->Groups.AddDefaulted_GetRef();
	NewGroup.Type = Type;
	NewGroup.Name = Name;
	NewGroup.Id = GetNewIdForGroup(NewGroup);

	return NewGroup;
}
//
//FCommandGroup* FCommandsManager::AddNewGroup(const FString& Name)
//{
//	return AddNewGroup(Name, EGroupType::Default);
//}


//bool FCommandsManager::RenameGroup(int Id, const FString& NewName)
//{
//	check(CommandGroups.IsValidIndex(Id));
//
//	CommandGroups[Id].Name = NewName;
//	CommandGroups[Id].Id = GetNewIdForGroup(CommandGroups[Id]);
//
//	return true;
//
//}

bool FCommandsManager::RenameGroup(FGuid Id, const FString& NewName)
{
	UCommandsContainer** FoundContainer = GroupToContainerMap.Find(Id);

	if (FoundContainer)
	{
		UCommandsContainer* Container = *FoundContainer;

		FCommandGroup* FoundGroup = Container->Groups.FindByKey<FGuid>(Id);

		FoundGroup->Name = NewName;

		OnDataRefreshed.ExecuteIfBound();

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
			NewGroup.Id = GetNewIdForGroup(NewGroup);


			Container->Groups.Insert(NewGroup, GroupIndex + 1);
		}
	}
	OnDataRefreshed.ExecuteIfBound();
}

FGuid FCommandsManager::GetNewIdForGroup(const FCommandGroup& Group)
{
	//FString NewGroupId = FString(Group.Name) + "_";

	//int NewId = 0;

	//FString TempName = NewGroupId + FString::FromInt(NewId);
	//while (CommandGroups.FindByKey<FString>(TempName))
	//{
	//	++NewId;
	//	TempName = NewGroupId + FString::FromInt(NewId);
	//}


	//return TempName;

	return FGuid::NewGuid();
}

bool FCommandsManager::SaveCommands()
{
	for (auto& Container : CommandsContainers)
	{
		if (Container->IsValidLowLevel())
		{
			const FString Path = Container->GetPackage()->GetPathName();
			UE_LOG(LogTemp, Warning, TEXT("Path to obj container %s : %s"), *Container->GetName(), *Path);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Object is not valid!"));
		}
		
	}


	//const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources") / TEXT("Commands.txt");

	//return FileHelper::SaveCommandFile(CommandsPath, CommandGroups);

	//for (auto Container : CommandsContainers)
	//{
	//	Container->Groups.Reset();
	//}
	//
	//for (const auto& Group : CommandGroups)
	//{
	//	UObject* ContainerObj = Group.ContainerSoftPtr.Get();
	//	if (ContainerObj)
	//	{
	//		UCommandsContainer* Container = StaticCast<UCommandsContainer*>(ContainerObj);
	//		if (Container)
	//		{
	//			FCommandGroup* FoundGroup = Container->Groups.FindByKey<FString>(Group.Id);
	//			if (FoundGroup)
	//			{
	//				FoundGroup->Commands = Group.Commands;
	//			}
	//			else
	//			{

	//			}
	//		}
	//	}
	//}



		//TArray<FAssetData> Assets;
//UAssetManager::Get().GetAssetRegistry().GetAssetsByClass(UCommandsContainer::StaticClass()->GetFName(), Assets);


//UE_LOG(LogTemp, Warning, TEXT("Asset count %d || %s"), Assets.Num(), *UCommandsContainer::StaticClass()->GetFName().ToString());

//for (auto& Asset : Assets)
//{
//	UPackage* Package = Asset.GetPackage();
//	//Package->MarkPackageDirty();


//	UObject* Resolved = Asset.GetAsset();

//	if (Resolved)
//	{
//		UCommandsContainer* Container = Cast< UCommandsContainer >(Resolved);
//		auto& Groups = CommandsManager.Get()->GetCommandGroups();

//		for (int i = 0; i < Groups.Num(); i++)
//		{
//			Container->Groups.Add(Groups[i]);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Not resolved!"));
//	}

//	UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());
//	// Construct a filename from long package name.
//	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
//	
//	//FString Path = FString::Printf(TEXT("%s%s%s"));

//	FSavePackageResultStruct OutStruct = Package->Save(Package, Asset.GetAsset(), EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *PackageFileName);
//	//UPackage::Save(Package, Resolved, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, Asset.);
//	//Resolved->PostEditChange();
//	
//	UE_LOG(LogTemp, Warning, TEXT("Result of save: %d | %lld"), OutStruct.Result, OutStruct.TotalFileSize);
//	
//}

	return true;
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
	FString ExecCommand = Out.Last();

	//ConsoleHistory.Commands.RemoveAll([&ExecCommand](FConsoleCommand& Command) { return Command.GetExec().Equals(ExecCommand, ESearchCase::IgnoreCase); });
	ConsoleHistory.Commands.Add(FConsoleCommand(ExecCommand));


	if (IsHistorySelected())
	{
		RebuildSharedArray();
	}
}

bool FCommandsManager::IsHistorySelected()
{
	return CurrentGroup == &ConsoleHistory ? true : false;
}

TArray<UCommandsContainer*> FCommandsManager::LoadAllContainers()
{
	TArray<FAssetData> Assets;
	UAssetManager::Get().GetAssetRegistry().GetAssetsByClass(UCommandsContainer::StaticClass()->GetFName(), Assets);

	TArray<UCommandsContainer*> Containers;

	UE_LOG(LogTemp, Warning, TEXT("Found %d assets %d"), Assets.Num());

	for (auto& Asset : Assets)
	{
		UObject* Resolved = Asset.GetAsset();

		if (Resolved)
		{
			UCommandsContainer* Container = Cast< UCommandsContainer >(Resolved);

			Containers.Add(Container);
		}
	}

	return Containers;
}

void FCommandsManager::SaveToAssets()
{

	if (ISourceControlModule::Get().IsEnabled())
	{
		TArray<UPackage*> PackageList;
		for (auto& Container : CommandsContainers)
		{
			PackageList.Add(Container->GetPackage());
		}

		FEditorFileUtils::PromptForCheckoutAndSave(PackageList, false, false);
	}
	else
	{

		TArray<UObject*> NotSavedAssets;
		// If disabled just go through and save everything
		for (auto& Container : CommandsContainers)
		{
			UPackage* Package = Container->GetPackage();

			Package->MarkPackageDirty();

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
	//FUnrealEdMisc::Get().RestartEditor(bWarn);

	//for (auto& Container : CommandsContainers)
	//{
	//	UPackage* Package = Container->GetPackage();

	//	Package->MarkPackageDirty();

	//	UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());

	//	// Construct a filename from long package name.
	//	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
	//
	//	//FString Path = FString::Printf(TEXT("%s%s%s"));
	//	
	//	//IPlatformFile::GetPlatformPhysical();

	//	

	//	//FEditorFileUtils::Source

	//	//FEditorFileUtils::CheckoutPackages()

	//	//UPackage::Save(Package, Resolved, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, Asset.);
	//	//Resolved->PostEditChange();
	//
	//	
	//}

	

	//TArray<FAssetData> Assets;
//UAssetManager::Get().GetAssetRegistry().GetAssetsByClass(UCommandsContainer::StaticClass()->GetFName(), Assets);


//UE_LOG(LogTemp, Warning, TEXT("Asset count %d || %s"), Assets.Num(), *UCommandsContainer::StaticClass()->GetFName().ToString());

//for (auto& Asset : Assets)
//{
//	UPackage* Package = Asset.GetPackage();
//	//Package->MarkPackageDirty();


//	UObject* Resolved = Asset.GetAsset();

//	if (Resolved)
//	{
//		UCommandsContainer* Container = Cast< UCommandsContainer >(Resolved);
//		auto& Groups = CommandsManager.Get()->GetCommandGroups();

//		for (int i = 0; i < Groups.Num(); i++)
//		{
//			Container->Groups.Add(Groups[i]);
//		}
//	}
//	else
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Not resolved!"));
//	}

//	UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());
//	// Construct a filename from long package name.
//	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
//	
//	//FString Path = FString::Printf(TEXT("%s%s%s"));

//	FSavePackageResultStruct OutStruct = Package->Save(Package, Asset.GetAsset(), EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *PackageFileName);
//	//UPackage::Save(Package, Resolved, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, Asset.);
//	//Resolved->PostEditChange();
//	
//	UE_LOG(LogTemp, Warning, TEXT("Result of save: %d | %lld"), OutStruct.Result, OutStruct.TotalFileSize);
//	
//}
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

	for (int i = 0; i < CommandsNum; i++)
	{
		CurrentCommandsShared.Add(MakeShareable(&CurrentGroup->Commands[i], FDeleterNot()));
	}
}

//It will not add wrong command to history
bool FCommandsManager::Execute(const FConsoleCommand& Command)
{


	//IModularFeatures::Get().RegisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), CmdExec.Get());

	//IConsoleCommandExecutor& CmdExec = IModularFeatures::Get().GetModularFeature<IConsoleCommandExecutor>("ConsoleCommandExecutor");

	//CmdExec.Exec()
	const FString& ExecCommand = Command.GetExec();

	bool SuccessExecuting = GEngine->Exec(GEngine->GetWorldContexts().Last().World(), *ExecCommand, *GLog);

	

	if (SuccessExecuting)
	{
		IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *ExecCommand);
	}
	else
	{

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


// no need to reparse every time, keep objects IConsoleObjects in memory 
void FCommandsManager::SetCurrentCommands(FCommandGroup& Group)
{
	CurrentGroup = &Group;

	//CurrentGroupId = Group.Id;

	RebuildSharedArray();
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
