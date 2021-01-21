// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandsManager.h"

#include "Misc/MessageDialog.h"

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
	FileHelper::LoadConsoleHistory(ConsoleHistory);
	
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

const TArray<TSharedPtr<FConsoleCommand>>& FCommandsManager::GetCurrentCommandsSharedPtr()
{
	CurrentCommandsShared.Empty();

	for (int i = 0; i < CurrentGroup->Commands.Num(); i++)
	{
		CurrentCommandsShared.Add(MakeShareable(&CurrentGroup->Commands[i], FDeleterNot()));
	}

	return CurrentCommandsShared;
}

const TArray<FConsoleCommand>& FCommandsManager::GetCurrentCommands()
{
	UE_LOG(LogTemp, Warning, TEXT("Test"));
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




void FCommandsManager::SetActiveGroup(int NewId)
{
	//index here should always be valid
	//check(!(NewId > 0 && NewId < CommandGroups.Num()));

	//const bool bValidIndex = NewId > 0 && NewId < CommandGroups.Num();
	//if (!bValidIndex)
	//{
	//	return;
	//}
	
	
	
	SetCurrentCommands(CommandGroups[NewId]);

}

void FCommandsManager::AddNewGroup(const FString& Name)
{
	FCommandGroup& NewGroup = CommandGroups.AddDefaulted_GetRef();
	NewGroup.Name = Name;
}

const FConsoleCommand& FCommandsManager::GetConsoleCommand(int Id)
{
	return CurrentGroup->Commands[Id];
}



bool FCommandsManager::ExecuteCurrentCommand(int Id)
{
	return Execute(CurrentGroup->Commands[Id]);
}

bool FCommandsManager::ExecuteCommand(const FConsoleCommand& Command)
{
	return false; // Execute(Command);
}

void FCommandsManager::RemoveGroup(int Id)
{
	check(CommandGroups.IsValidIndex(Id));

	if (CurrentGroup == &CommandGroups[Id])
	{
		UE_LOG(LogTemp, Warning, TEXT("Its the same!"));
		SetCurrentCommands(ConsoleHistory);
	}

	CommandGroups.RemoveAt(Id);

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

//It will not add wrong command to history
bool FCommandsManager::Execute(FConsoleCommand& Command)
{


	//IModularFeatures::Get().RegisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), CmdExec.Get());

	//IConsoleCommandExecutor& CmdExec = IModularFeatures::Get().GetModularFeature<IConsoleCommandExecutor>("ConsoleCommandExecutor");

	//CmdExec.Exec()

	if (GEngine->Exec(GEngine->GetWorldContexts().Last().World(), *Command.Command, *GLog))
	{
		IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *Command.Command);
		ConsoleHistory.Commands.Add(FConsoleCommand(Command.Command));

		Command.IsValid = true;

		return true;
	}

	Command.IsValid = false;

	UE_LOG(LogTemp, Warning, TEXT("Wrong command! %s"), *Command.Command);
	return false;

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
		if (!AllCommands.Commands.FindByKey<FString>(Command.Name))
		{
			Command.IsValid = false;
		}
	}
}


// no need to reparse every time, keep objects IConsoleObjects in memory 
void FCommandsManager::SetCurrentCommands(FCommandGroup& Group)
{
	CurrentGroup = &Group;
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

	for (auto& Command : LocalCommands)
	{
		AllCommands.Commands.Add(FConsoleCommand(Command));
	}


	const FString Path = FPaths::GeneratedConfigDir() + TEXT("Console.txt");
	
	FileHelper::DumpAllCommands(Path, LocalCommands);
}


#undef LOCTEXT_NAMESPACE
