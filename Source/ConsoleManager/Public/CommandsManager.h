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
DECLARE_DELEGATE(FOnGroupsRefresh);
DECLARE_DELEGATE(FOnCommandsRefresh);

/**
 * Class made to avoid deletion of ConsoleCommand objects when TSharedPtr goes out of scope as we are using very unsafe method
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
	~FCommandsManager();

	void Initialize();
	void Initialize(const TArray<UCommandsContainer*>& Containers);

	
	FORCEINLINE const TArray<TSharedPtr<FConsoleCommand>>& GetCurrentSharedCommands() const { return CurrentCommandsShared; }
	FORCEINLINE const FCommandGroup& GetCurrentCommandGroup() const { return *CurrentGroup; };
	FORCEINLINE const TArray<FConsoleCommand>& GetCurrentCommands() const { return CurrentGroup->Commands; }
	FORCEINLINE const FCommandGroup* GetAllCommands() const { return &AllCommands; }
	FORCEINLINE const FCommandGroup* GetHistory() const { return &ConsoleHistory; }
	FORCEINLINE const FCommandGroup& GetSnapshot() const { return Snapshot; }

	const TArray<TPair<FString, FGuid>> GetGroupList();
	bool SetActiveHistory();
	bool SetActiveAllCommands();
	bool SetActiveSnapshot();

	const FCommandGroup* GetGroupById(const FGuid& Id);
	
	bool SetActiveGroup(FGuid Id);

	void CreateNewGroup(const FString& Name, UCommandsContainer* Container);

	// Creates new group with given commands in the same container as given group 
	void CreateNewGroup(const FString& Name, UCommandsContainer* Container, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void CreateSnapshotCVars();
	void RevertSnapshotCVars();

	void RemoveGroup(FGuid Id);
	bool RenameGroup(FGuid Id, const FString& NewName);
	void DuplicateGroup(FGuid Id);

	void ReorderCommandInCurrentGroup(int32 CurrentId, int32 NewId);
	void DuplicateCommand(int32 Id);
	void RemoveCommands(TArray<int32> Ids);
	void SetNoteCommand(int32 Id, const FString& NewNote);

	void AddCommandsToCurrentGroup(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void AddCommandsToGroup(FGuid Id, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void UpdateCurrentEngineValue(const FConsoleCommand& Command);

	void ReplaceCommandInCurrentGroup(int32 Id, FConsoleCommand& NewCommand);

	const FConsoleCommand& GetConsoleCommand(int Id);

	bool ExecuteCommand(const FConsoleCommand& Command);
	bool ExecuteCommand(FConsoleCommand& Command);
	void ExecuteMultipleCommands(TArray<TSharedPtr<FConsoleCommand>> Commands);
	void ExecuteGroup(const FGuid& Id);
	bool ExecuteGroup(UCommandsContainer* Container, const FString& Name);

	void UpdateHistory();

	void SetHistoryBufferSize(int32 NewSize) { HistoryBufferSize = NewSize; }


	const TArray<UCommandsContainer*>& GetCommandsContainers() { return CommandsContainers; };

	void SaveToAssets();
	FOnDataRefreshed OnDataRefreshed;
	FOnGroupsRefresh OnGroupsRefresh;
	FOnCommandsRefresh OnCommandsRefresh;

	void Refresh();

	void ShouldLoadAllContainers(bool Val);
private:

	// Function called when console sink is called (on variable change), can be blocked by bSinkBlocked
	void VariableChanged();

	void Initialize_Internal(const TArray<UCommandsContainer*>& Containers);

	FCommandGroup& AddNewGroup_Internal(const FString& Name, UCommandsContainer* Container, EGroupType Type = EGroupType::Default);

	TArray<UCommandsContainer*> LoadAllContainers();


	void AddCommandsToGroup(FCommandGroup* Group, TArray<TSharedPtr<FConsoleCommand>> Commands);

	void LoadConsoleHistory();

	/**
	 * @brief Refresh pointers to current displayed commands.
	 *		  This should be called whenever anything in current commands change
	*/
	void RebuildSharedArray();

	//void ContainerChanged();
	void ContainerChanged(UCommandsContainer* Container = nullptr);

	void SetCurrentCommands(FCommandGroup& Group);


	void ContainerBeingDestroyed(UCommandsContainer* Container);
	void ContainerRenamed(UCommandsContainer* Container);


	bool Execute_Internal(const FConsoleCommand& Command, bool UpdateHistory = true);
	bool ExecuteGroup_Internal(const FCommandGroup& Group);

	void ValidateCommands(TArray<FConsoleCommand>& Commands);

	FGuid GetNewIdForGroup();

	void DumpAllCommands();

	void DisplayNotification(FText Text, SNotificationItem::ECompletionState State);

	// Return nullptr if not found
	FCommandGroup* GetGroup(FGuid Id);

	/** How many commands to keep in history */
	int32 HistoryBufferSize = 64;

	/** Assets being watched */
	TArray<UCommandsContainer*> CommandsContainers;
	// We should generate new array every time current group commands might be resized! (When adding, removing elements)
	TArray<TSharedPtr<FConsoleCommand>> CurrentCommandsShared;


	// Current active group to be displayed (this always needs to be set)
	FCommandGroup* CurrentGroup;

	// Current container from which group is selected (this always needs to be set)
	UCommandsContainer* CurrentContainer;

	// Groups for always existing commands
	FCommandGroup AllCommands;
	FCommandGroup ConsoleHistory;
	FCommandGroup Snapshot;

	/** Utility map to easily find in which container group lies */
	TMap<FGuid, UCommandsContainer*> GroupToContainerMap;

	// Keep reference to one notification item to avoid spamming
	TSharedPtr<SNotificationItem> NotificationItem;


	//This handle allows to catch any changes to cvars
	FConsoleVariableSinkHandle Handle;

	bool bSinkBlocked = false;

	bool bLoadAllContainers = false;
};
