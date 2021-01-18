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

	//const FString ConfigPath = FPaths::GeneratedConfigDir() + TEXT("Commands.ini");

	const FString CommandsPath = IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources") / TEXT("Commands.txt");


	FileHelper::ReadCommandFile(CommandsPath, CommandGroups);
	FileHelper::LoadConsoleHistory(ConsoleHistory);


	FileHelper::PrintGroups_Debug(CommandGroups);

	FConsoleCommand Command;
	Command.Command = "R.lol";

	CurrentCommands.Push(Command);
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

}

const TArray<FConsoleCommand>& FCommandsManager::GetCurrentCommands()
{
	UE_LOG(LogTemp, Warning, TEXT("Test"));
	return CurrentCommands;
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
