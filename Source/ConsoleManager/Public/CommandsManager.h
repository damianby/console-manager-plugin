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

class FDeleterNot
{
public:
	void operator() (void*) {};
};


class CONSOLEMANAGER_API FCommandsManager
{
public:

	FCommandsManager();

	bool Initialize(const FString& Path);

	void Refresh();

	const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentCommandsSharedPtr();
	FORCEINLINE const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentCommandsSharedPtr_Cache() { return CurrentCommandsShared; }
	const TArray<FConsoleCommand>& GetCurrentCommands();
	const TArray<FString> GetGroupList();

	FORCEINLINE const TArray<FCommandGroup>& GetCommandGroups() { return CommandGroups; };

	void SetActiveGroup(int NewId);


	void AddNewGroup(const FString& Name);

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCurrentCommand(int Id);
	bool ExecuteCommand(const FConsoleCommand& Command);

	void RemoveGroup(int Id);
	bool RenameGroup(int Id, const FString& NewName);
	void DuplicateGroup(int Id);

	bool SaveCommands();
private:

	TArray<TSharedPtr<FConsoleCommand>> CurrentCommandsShared;

	void SetCurrentCommands(FCommandGroup& Group);

	FCommandGroup ConsoleHistory;

	TArray<FCommandGroup> CommandGroups;

	FCommandGroup* CurrentGroup;

	bool Execute(FConsoleCommand& Command);

	void ValidateCommands(TArray<FConsoleCommand>& Commands);

	FString GetNewIdForGroup(const FCommandGroup& Group);

	void DumpAllCommands();

	FCommandGroup AllCommands;
	FString CurrentGroupId;
};
