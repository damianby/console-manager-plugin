// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Parse.h"

#include "CommandStructs.h"

class UCommandsContainer;

/**
 * 
 */

/**
 * Class made to avoid deletion of ConsoleCommand objects when TSharedPtr goes out of scope
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

	bool InitializeFromContainers(TArray<UCommandsContainer*> Containers);

	void Refresh();

	const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentSharedCommands();
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



	void AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void UpdateCurrentEngineValue(const FConsoleCommand& Command);

	void ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand);

	const FCommandGroup* GetAllCommands();
	FCommandGroup* GetGroupById(const FString& Id);

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCommand(const FConsoleCommand& Command);
	bool ExecuteCommand(FConsoleCommand& Command);
	void ExecuteMultipleCommands(TArray<TSharedPtr<FConsoleCommand>> Commands);


	FCommandGroup* AddNewGroup(const FString& Name, EGroupType Type);
	FCommandGroup* AddNewGroup(const FString& Name);
	void RemoveGroup(int Id);
	bool RenameGroup(int Id, const FString& NewName);
	void DuplicateGroup(int Id);

	bool SaveCommands();

	void UpdateHistory();

	bool IsHistorySelected();

	void SetHistoryBufferSize(int32 NewSize) { HistoryBufferSize = NewSize; }
private:

	void LoadAllAssets();
	void SaveToAssets();


	void LoadConsoleHistory();

	/**
	 * @brief Refresh pointers to current displayed commands.
	 *		  This should be called whenever anything in current commands change
	*/
	void RebuildSharedArray();

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

	/** How many commands to keep in history */
	int32 HistoryBufferSize = 64;

	/** Assets being watched */
	TArray<UCommandsContainer*> CommandsContainers;
};
