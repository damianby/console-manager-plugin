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

struct FCommandTransaction
{
	
};


class CONSOLEMANAGER_API FCommandsManager
{
public:

	FCommandsManager();

	bool Initialize(const FString& Path);

	void Refresh();

	const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentCommandsSharedPtr();
	FORCEINLINE const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentCommandsSharedPtr_Cache() { return CurrentCommandsShared; }
	FORCEINLINE const FCommandGroup& GetCurrentCommandGroup() const { return *CurrentGroup; };
	const TArray<FConsoleCommand>& GetCurrentCommands();
	const TArray<FString> GetGroupList();

	FORCEINLINE const TArray<FCommandGroup>& GetCommandGroups() { return CommandGroups; };

	bool SetActiveGroup(int NewId);

	void ReorderCommandInCurrentGroup(int32 CurrentId, int32 NewId);
	void DuplicateCommand(int32 Id);
	void RemoveCommands(TArray<int32> Ids);

	bool SetActiveHistory();
	bool SetActiveAllCommands();

	FCommandGroup* AddNewGroup(const FString& Name, EGroupType Type);
	FCommandGroup* AddNewGroup(const FString& Name);

	void AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void UpdateCurrentEngineValue(const FConsoleCommand& Command);

	void ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand);

	const FCommandGroup* GetAllCommands();
	FCommandGroup* GetGroupById(const FString& Id);

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCommand(FConsoleCommand& Command);
	void ExecuteMultipleCommands(TArray<TSharedPtr<FConsoleCommand>> Commands);

	void RemoveGroup(int Id);
	bool RenameGroup(int Id, const FString& NewName);
	void DuplicateGroup(int Id);

	bool SaveCommands();
private:

	// We should generate new array every time this array might be resized! (When adding, removing elements)
	TArray<TSharedPtr<FConsoleCommand>> CurrentCommandsShared;

	void SetCurrentCommands(FCommandGroup& Group);

	FCommandGroup ConsoleHistory;

	TArray<FCommandGroup> CommandGroups;

	FCommandGroup* CurrentGroup;

	bool Execute(const FConsoleCommand& Command);

	void ValidateCommands(TArray<FConsoleCommand>& Commands);

	FString GetNewIdForGroup(const FCommandGroup& Group);

	void DumpAllCommands();

	FCommandGroup AllCommands;
	FString CurrentGroupId;

};
