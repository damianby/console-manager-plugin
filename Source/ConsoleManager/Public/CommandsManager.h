// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Parse.h"

#include "CommandStructs.h"
#include "Widgets/Notifications/SNotificationList.h"

class UCommandsContainer;

DECLARE_DELEGATE(FOnDataRefreshed);


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

	void Initialize();
	void Initialize(TArray<UCommandsContainer*> Containers);

	void Refresh();

	const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentSharedCommands();
	FORCEINLINE const FCommandGroup& GetCurrentCommandGroup() const { return *CurrentGroup; };
	const TArray<FConsoleCommand>& GetCurrentCommands();
	const TArray<TPair<FString, FGuid>> GetGroupList();



	FORCEINLINE const TArray<FCommandGroup>& GetCommandGroups() { return CommandGroups; };

	//bool SetActiveGroup(int NewId);
	bool SetActiveGroup(FGuid Id);

	void ReorderCommandInCurrentGroup(int32 CurrentId, int32 NewId);
	void DuplicateCommand(int32 Id);
	void RemoveCommands(TArray<int32> Ids);

	bool SetActiveHistory();
	bool SetActiveAllCommands();
	bool SetActiveSnapshot();



	void AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void AddCommandsToGroup(FGuid Id, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void UpdateCurrentEngineValue(const FConsoleCommand& Command);

	void ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand);

	const FCommandGroup* GetAllCommands();
	const FCommandGroup* GetHistory();
	const FCommandGroup* GetSnapshot();
	const FCommandGroup* GetGroupById(const FGuid& Id);

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCommand(const FConsoleCommand& Command);
	bool ExecuteCommand(FConsoleCommand& Command);
	void ExecuteMultipleCommands(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void ExecuteGroup(const FGuid& Id);

	void CreateNewGroup(const FString& Name, UCommandsContainer* Container);

	// Creates new group with given commands in the same container as given group 
	void CreateNewGroup(const FString& Name, UCommandsContainer* Container, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void CreateSnapshotCVars(const FString& Name, UCommandsContainer* Container);

	void RemoveGroup(FGuid Id);
	bool RenameGroup(FGuid Id, const FString& NewName);
	void DuplicateGroup(FGuid Id);



	bool SaveCommands();

	void UpdateHistory();

	bool IsHistorySelected();

	void SetHistoryBufferSize(int32 NewSize) { HistoryBufferSize = NewSize; }


	const TArray<UCommandsContainer*>& GetCommandsContainers() { return CommandsContainers; };

	void SaveToAssets();
	FOnDataRefreshed OnDataRefreshed;
private:


	void Initialize_Internal(TArray<UCommandsContainer*> Containers);

	FCommandGroup& AddNewGroup_Internal(const FString& Name, UCommandsContainer* Container, EGroupType Type = EGroupType::Default);

	TArray<UCommandsContainer*> LoadAllContainers();



	void AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands);

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
	FCommandGroup Snapshot;

	TArray<FCommandGroup> CommandGroups;

	FCommandGroup* CurrentGroup;

	bool Execute(const FConsoleCommand& Command);

	void ValidateCommands(TArray<FConsoleCommand>& Commands);

	FGuid GetNewIdForGroup(const FCommandGroup& Group);

	void DumpAllCommands();

	void DisplayNotification(FText Text, SNotificationItem::ECompletionState State);

	// Return nullptr if not found
	FCommandGroup* GetGroup(FGuid Id);

	FCommandGroup AllCommands;
	FString CurrentGroupId;

	/** How many commands to keep in history */
	int32 HistoryBufferSize = 64;

	/** Assets being watched */
	TArray<UCommandsContainer*> CommandsContainers;


	TMap<FGuid, UCommandsContainer*> GroupToContainerMap;

	TSharedPtr<SNotificationItem> NotificationItem;
};
