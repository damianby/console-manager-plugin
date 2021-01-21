// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandsManager.h"

static inline bool IsWhiteSpace(TCHAR Value) { return Value == TCHAR(' '); }

static const TCHAR* GetSetByTCHAR(EConsoleVariableFlags InSetBy)
{
	EConsoleVariableFlags SetBy = (EConsoleVariableFlags)((uint32)InSetBy & ECVF_SetByMask);

	switch (SetBy)
	{
#define CASE(A) case ECVF_SetBy##A: return TEXT(#A);
		// Could also be done with enum reflection instead
		CASE(Constructor)
			CASE(Scalability)
			CASE(GameSetting)
			CASE(ProjectSetting)
			CASE(DeviceProfile)
			CASE(SystemSettingsIni)
			CASE(ConsoleVariablesIni)
			CASE(Commandline)
			CASE(Code)
			CASE(Console)
#undef CASE
	}
	return TEXT("<UNKNOWN>");
}

FCommandsManager::FCommandsManager()
{

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
	
	FCommandGroup* CurrentGroupByName = CommandGroups.FindByKey<FString>(CurrentCommandGroupName);
	if (CurrentGroupByName)
	{
		SetCurrentCommands(*CurrentGroupByName);
	}
	else
	{
		SetCurrentCommands(ConsoleHistory);
	}
	
}

}

const TArray<FConsoleCommand>& FCommandsManager::GetCurrentCommands()
{
	UE_LOG(LogTemp, Warning, TEXT("Test"));
	return CurrentCommands->Commands;
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

	ActiveGroupId = NewId;
	
	SetCurrentCommands(CommandGroups[ActiveGroupId]);
	RefreshCurrentTrackedCommands();

	DumpAllCommands();
}

void FCommandsManager::AddNewGroup(const FString& Name)
{
	FCommandGroup& NewGroup = CommandGroups.AddDefaulted_GetRef();
	NewGroup.Name = Name;
}

const FConsoleCommand& FCommandsManager::GetConsoleCommand(int Id)
{
	return CurrentCommands->Commands[Id];
}



bool FCommandsManager::ExecuteCurrentCommand(int Id)
{
	return Execute(CurrentCommands->Commands[Id]);
}

bool FCommandsManager::ExecuteCommand(const FConsoleCommand& Command)
{
	return false; // Execute(Command);
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

		RefreshCommand(Command);

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

void FCommandsManager::RefreshCommand(FConsoleCommand& Command)
{
	const TCHAR* It = *Command.Command;

	FString Param1 = GetTextSection(It);
	if (Param1.IsEmpty())
	{
		//return false;
	}

	IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Param1);

	if (Obj)
	{
		Command.SetBy = GetSetByTCHAR(Obj->GetFlags());

		IConsoleVariable* CVar = Obj->AsVariable();
		if (CVar)
		{
			Command.Value = CVar->GetString();
		}
	}
}

void FCommandsManager::RefreshCurrentTrackedCommands()
{
	TArray<FConsoleCommand>& CommandCollection = CurrentCommands->Commands;

	for (auto& Command : CommandCollection)
	{
		RefreshCommand(Command);
	}
}

// no need to reparse every time, keep objects IConsoleObjects in memory 
void FCommandsManager::SetCurrentCommands(FCommandGroup& Group)
{
	if (Group.bInitiallySet)
	{
		CurrentCommands = &Group;
		RefreshCurrentTrackedCommands();
		return;
	}
		
	
	TArray<FConsoleCommand>& CommandCollection = Group.Commands;


	for (int i = 0; i < CommandCollection.Num(); i++)
	{
		FConsoleCommand& Command = CommandCollection[i];

		//we assume that is valid, we will later check that if its used
		Command.IsValid = true;

		const TCHAR* It = *Command.Command;

		FString Param1 = GetTextSection(It);
		if (Param1.IsEmpty())
		{
			//return false;
		}


		IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Param1);
	
		if (Obj)
		{
			//Object exists so its safe to assume it has any kind of value
			Command.InputType = EConsoleCommandInputType::Value;

			Command.Name = Param1;

			Command.SetBy = GetSetByTCHAR(Obj->GetFlags());

			IConsoleCommand* CCmd = Obj->AsCommand();
			IConsoleVariable* CVar = Obj->AsVariable();


			if (CCmd)
			{
			}
			else if (CVar)
			{

				Command.Value = CVar->GetString();

				if (CVar->IsVariableBool())
				{
					Command.Type = "Bool";
				}
				else if (CVar->IsVariableFloat())
				{
					Command.Type = "Float";
				}
				else if (CVar->IsVariableInt())
				{
					Command.Type = "Int";
				}
				else if (CVar->IsVariableString())
				{
					Command.Type = "String";
				}
			}

		}
		else
		{
			//Its engine command?
			Command.InputType = EConsoleCommandInputType::None;

			Command.Name = Command.Command;
		}

		UE_LOG(LogTemp, Warning, TEXT("Initial: %s, %s, %s, %d"), *Command.Name, *Command.Value, *Command.Command, Command.InputType);

	}
	for (const auto& ConsoleCommand : CommandCollection) {

		//IConsoleManager::Get().ForEachConsoleObjectThatContains(FConsoleObjectVisitor::CreateLambda(OnConsoleVariable), *InputTextStr);

		const TCHAR* It = *ConsoleCommand.Command;

		FString Param1 = GetTextSection(It);
		if (Param1.IsEmpty())
		{
			//return false;
		}

		FString HelpString;

		IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Param1);
		if (Obj)
		{
			if (Obj->TestFlags(ECVF_Unregistered))
			{
				//Handle unregistered!
				UE_LOG(LogTemp, Warning, TEXT("No test flag"));

				continue;
			}

			IConsoleCommand* CCmd = Obj->AsCommand();
			IConsoleVariable* CVar = Obj->AsVariable();



			if (CCmd)
			{
				HelpString = "Command:\n";
			}
			else if (CVar)
			{
				HelpString = "Variable:\n";


			}


			if (Obj != nullptr)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Value is: %s\n Set by: %s"), *Obj->);
				UE_LOG(LogTemp, Warning, TEXT("Set by: %s"), Obj->GetHelp());

				HelpString += Obj->GetHelp();

			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DIdnt find console command!"));
			}
		}

		

	}
	Group.bInitiallySet = true;

	CurrentCommands = &Group;
	RefreshCurrentTrackedCommands();


		/*ButtonList->AddSlot()[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(3.0f, 1.0f)[
					SNew(SButton)
						.ToolTipText(FText::FromString(HelpString))
						.ButtonColorAndOpacity(FLinearColor(1, 1, 1, .9))[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.Padding(1.0f, 3.0f)[
									SNew(STextBlock)
										.Text(FText::FromString(ConsoleCommand))
										.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 9))
								]
						]
						.OnClicked_Lambda([ConsoleCommand, this]()-> FReply {
										GEngine->Exec(GEngine->GetWorldContexts().Last().World(), *ConsoleCommand, *GLog);
										IConsoleManager::Get().AddConsoleHistoryEntry(TEXT(""), *ConsoleCommand);*/
}




FString FCommandsManager::GetTextSection(const TCHAR*& It)
{
	FString ret;

	while (*It)
	{
		if (IsWhiteSpace(*It))
		{
			break;
		}

		ret += *It++;
	}

	while (IsWhiteSpace(*It))
	{
		++It;
	}

	return ret;
}

void FCommandsManager::DumpAllCommands()
{
	TSet<FString>& LocalCommands = AllCommands;

	FConsoleObjectVisitor Visitor;
	Visitor.BindLambda([&LocalCommands](const TCHAR* Name, IConsoleObject* Obj) {

		LocalCommands.Add(Name);
	});

	IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(Visitor);

	LocalCommands.Sort(TLess<FString>());


	const FString Path = FPaths::GeneratedConfigDir() + TEXT("Console.txt");
	FileHelper::DumpAllCommands(Path, LocalCommands);

}