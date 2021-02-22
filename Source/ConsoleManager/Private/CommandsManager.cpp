// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "CommandsManager.h"

#include "Misc/MessageDialog.h"

#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

FCommandsManager::FCommandsManager()
{
	DumpAllCommands();

	Refresh();

	FileHelper::PrintGroups_Debug(CommandGroups);

	SetCurrentCommands(ConsoleHistory);
}

bool FCommandsManager::Initialize(const FString& Path)
{
	return true;
}

void FCommandsManager::Refresh()
{
	const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources") / TEXT("Commands.txt");

	FileHelper::ReadCommandFile(CommandsPath, CommandGroups);


	LoadConsoleHistory();

	
	ValidateCommands(ConsoleHistory.Commands);
	for (auto& Group : CommandGroups)
	{
		ValidateCommands(Group.Commands);
	}

	FCommandGroup* CurrentGroupByName = CommandGroups.FindByKey<FString>(CurrentGroupId);
	if (CurrentGroupByName)
	{
		SetCurrentCommands(*CurrentGroupByName);
	}
	else
	{
		SetCurrentCommands(ConsoleHistory);
	}
}

const TArray<TSharedPtr<FConsoleCommand>>& FCommandsManager::GetCurrentSharedCommands()
{
	return CurrentCommandsShared;
}

const TArray<FConsoleCommand>& FCommandsManager::GetCurrentCommands()
{
	return CurrentGroup->Commands;
}

const TArray<FString> FCommandsManager::GetGroupList()
{
	TArray<FString> Groups;

	for (const FCommandGroup& Group : CommandGroups)
	{
		Groups.Add(Group.Name);
	}

	return Groups;
}




bool FCommandsManager::SetActiveGroup(int NewId)
{
	check(CommandGroups.IsValidIndex(NewId));
	
	//if (!CurrentGroup->Id.Equals(CommandGroups[NewId].Id))
	//{
	SetCurrentCommands(CommandGroups[NewId]);
	return true;
	//}

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

FCommandGroup* FCommandsManager::AddNewGroup(const FString& Name, EGroupType Type)
{
	FCommandGroup& NewGroup = CommandGroups.AddDefaulted_GetRef();
	NewGroup.Type = Type;
	NewGroup.Name = Name;

	return &NewGroup;
}

FCommandGroup* FCommandsManager::AddNewGroup(const FString& Name)
{
	return AddNewGroup(Name, EGroupType::Default);
}

void FCommandsManager::AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	AddCommandsToGroup(CurrentGroup, Commands);
}

void FCommandsManager::AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	check(Group);

	if (Group->bIsEditable)
	{
		for (const auto& Command : Commands)
		{
			Group->Commands.Add(*Command.Get());
			FConsoleCommand& NewCommand = Group->Commands.Last();

			if (NewCommand.GetValue().IsEmpty())
			{
				NewCommand.SetValue(NewCommand.GetCurrentValue());
			}
		}
	}

	RebuildSharedArray();
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

FCommandGroup* FCommandsManager::GetGroupById(const FString& Id)
{
	FCommandGroup* FoundGroup = CommandGroups.FindByKey<FString>(Id);

	check(FoundGroup);

	return FoundGroup;
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

	FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("%s %s"), *ExecCommand, *ExecStateDisplay)));
	Info.bFireAndForget = true;
	Info.ExpireDuration = 3.0f;
	Info.FadeOutDuration = 2.0f;
	Info.FadeInDuration = 0.5f;
	Info.bUseSuccessFailIcons = true;
	//Info.HyperlinkText = FText::FromString(FString("Go to element"));
	//Info.Hyperlink = FSimpleDelegate::CreateLambda(
	//	[=]()
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Go to item!"));
	//	}
	//);

	TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SuccessExecuting ? SNotificationItem::ECompletionState::CS_Success : SNotificationItem::ECompletionState::CS_Fail);
	//OperationInProgressNotification = FSlateNotificationManager::Get().AddNotification(Info);


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

		FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("%d/%d commands executed succesfully!\nCommands failed:\n%s"), Commands.Num() - Errors.Num(), Commands.Num(), *Result)));
		Info.bFireAndForget = true;
		Info.ExpireDuration = 3.0f;
		Info.FadeOutDuration = 2.0f;
		Info.FadeInDuration = 0.5f;
		Info.bUseSuccessFailIcons = true;

		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_None);
	}
	else
	{

		FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("%d commands executed succesfully!"), Commands.Num())));
		Info.bFireAndForget = true;
		Info.ExpireDuration = 3.0f;
		Info.FadeOutDuration = 2.0f;
		Info.FadeInDuration = 0.5f;
		Info.bUseSuccessFailIcons = true;

		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::ECompletionState::CS_Success);
	}

}

void FCommandsManager::RemoveGroup(int Id)
{
	check(CommandGroups.IsValidIndex(Id));

	if (CurrentGroup == &CommandGroups[Id])
	{
		SetCurrentCommands(AllCommands);
	}

	CommandGroups.RemoveAt(Id, 1, false);

}

bool FCommandsManager::RenameGroup(int Id, const FString& NewName)
{
	check(CommandGroups.IsValidIndex(Id));

	CommandGroups[Id].Name = NewName;
	CommandGroups[Id].Id = GetNewIdForGroup(CommandGroups[Id]);

	return true;

}

void FCommandsManager::DuplicateGroup(int Id)
{
	check(CommandGroups.IsValidIndex(Id));

	FCommandGroup NewGroup = CommandGroups[Id];
	NewGroup.Name = NewGroup.Name + "_Copy";
	NewGroup.Id = GetNewIdForGroup(NewGroup);
	
	CommandGroups.Insert(NewGroup, Id + 1);
}

FString FCommandsManager::GetNewIdForGroup(const FCommandGroup& Group)
{
	FString NewGroupId = FString(Group.Name) + "_";

	int NewId = 0;

	FString TempName = NewGroupId + FString::FromInt(NewId);
	while (CommandGroups.FindByKey<FString>(TempName))
	{
		++NewId;
		TempName = NewGroupId + FString::FromInt(NewId);
	}
	return TempName;
}

bool FCommandsManager::SaveCommands()
{
	const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources") / TEXT("Commands.txt");

	return FileHelper::SaveCommandFile(CommandsPath, CommandGroups);
}

void FCommandsManager::UpdateHistory()
{
	TArray<FString> Out;
	IConsoleManager::Get().GetConsoleHistory(*FString(), Out);
	
	// limit size to avoid a ever growing file
	while (ConsoleHistory.Commands.Num() > HistoryBufferSize)
	{
		ConsoleHistory.Commands.RemoveAt(0);
	}
	FString ExecCommand = Out.Last();

	ConsoleHistory.Commands.RemoveAll([&ExecCommand](FConsoleCommand& Command) { return Command.GetExec().Equals(ExecCommand, ESearchCase::IgnoreCase); });
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

void FCommandsManager::LoadConsoleHistory()
{
	ConsoleHistory.Name = "History";
	ConsoleHistory.Id = FGuid::NewGuid().ToString();
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

	CurrentGroupId = Group.Id;

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
	AllCommands.Id = FGuid::NewGuid().ToString();
	AllCommands.bIsEditable = false;
	AllCommands.Type = EGroupType::AllCommands;

	for (auto& Command : LocalCommands)
	{
		AllCommands.Commands.Add(FConsoleCommand(Command));
	}


	const FString Path = FPaths::GeneratedConfigDir() + TEXT("Console.txt");
	
	FileHelper::DumpAllCommands(Path, LocalCommands);
}


#undef LOCTEXT_NAMESPACE
