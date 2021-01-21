// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Parse.h"
#include "CommandStructs.h"

/**
 * 
 */
class CONSOLEMANAGER_API FCommandsManager
{
public:

	FCommandsManager();

	bool Initialize(const FString& Path);

	void Refresh();

	const TArray<FConsoleCommand>& GetCurrentCommands();
	const TArray<FString> GetGroupList();

	void SetActiveGroup(int NewId);
	int GetActiveGroupId() { return ActiveGroupId; };

	void AddNewGroup(const FString& Name);

	void RefreshCurrentTrackedCommands();

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCurrentCommand(int Id);
	bool ExecuteCommand(const FConsoleCommand& Command);

	bool SaveCommands();
private:

	void SetCurrentCommands(FCommandGroup& Group);

	FCommandGroup ConsoleHistory;

	TArray<FCommandGroup> CommandGroups;

	FCommandGroup* CurrentGroup;

	bool Execute(FConsoleCommand& Command);

	void RefreshCommand(FConsoleCommand& Command);

	FString GetNewIdForGroup(const FCommandGroup& Group);

	void DumpAllCommands();

	FString GetTextSection(const TCHAR*& It);


	TSet<FString> AllCommands;
	FString CurrentGroupId;
};
