// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
#include "CommandsManager.h"
#include "SGroupButton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SScrollBox.h"
/**
 * 
 */

#include <chrono>

class UCommandsContainer;

class DragNDrop : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(DragNDrop, FDragDropOperation)

	int32 Id = 0;
	TSharedPtr<FConsoleCommand> Command;
	TSharedPtr<FCommandsManager> Manager;
};


DECLARE_DELEGATE_OneParam(FOnExecuteCommand, const FConsoleCommand&)
DECLARE_DELEGATE_OneParam(FOnSimpleCommand, TSharedPtr<FConsoleCommand>)
DECLARE_DELEGATE_TwoParams(FOnCommandValueEdit, TSharedPtr<FConsoleCommand>, int32)

class SConsoleCommandListRow : public SMultiColumnTableRow<TSharedPtr<FConsoleCommand>>
{
public:

    SLATE_BEGIN_ARGS(SConsoleCommandListRow) {}

		SLATE_EVENT(FOnDragDetected, OnDragDetected)
		SLATE_EVENT(FOnTableRowDragEnter, OnDragEnter)
		SLATE_EVENT(FOnTableRowDragLeave, OnDragLeave)
		SLATE_EVENT(FOnTableRowDrop, OnDrop)
		SLATE_EVENT(FOnExecuteCommand, OnExecuteCommand)
		SLATE_EVENT(FOnSimpleCommand, OnEngineValueEditing)
		SLATE_EVENT(FOnCommandValueEdit, OnCommandValueEditing)
		SLATE_ARGUMENT(TSharedPtr<FConsoleCommand>, Item)
		SLATE_ARGUMENT(bool, bIsValid)
		SLATE_ARGUMENT(bool, bIsEditable)
		SLATE_ARGUMENT(bool, bDisplayIcons)
    SLATE_END_ARGS()
			 

public:

	void SetOnDragDetected(FOnDragDetected Delegate)
	{
		OnDragDetected_Handler = Delegate;
	}

	void SetOnTableRowDrop(FOnTableRowDrop Delegate)
	{
		OnDrop_Handler = Delegate;
	}

	void SetOnCanAcceptDrop(FOnCanAcceptDrop Delegate)
	{
		OnCanAcceptDrop = Delegate;
	}

	void SetOnAcceptDrop(FOnAcceptDrop Delegate)
	{
		OnAcceptDrop = Delegate;
	}

	void SetOnExecuteCommand(FOnExecuteCommand Delegate)
	{
		OnExecuteCommand = Delegate;
	}

	void SetOnEngineValueEditing(FOnSimpleCommand Delegate)
	{
		OnEngineValueEditing = Delegate;
	}

	void SetOnCommandValueEdit(FOnCommandValueEdit Delegate)
	{
		OnCommandValueEdit = Delegate;
	}

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
    {
        Item = InArgs._Item;
		bIsValid = InArgs._bIsValid;
		bIsEditable = InArgs._bIsEditable;
		bDisplayIcons = InArgs._bDisplayIcons;
		//static FTableRowStyle NewStyle = FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");
		//NewStyle.DropIndicator_Onto = NewStyle.;
		
		const FTableRowStyle* StyleRow = &FConsoleManagerStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");


		FTableRowArgs Args;

		Args.Padding(FMargin(0, 2.0f));
		Args.Style(StyleRow);

		bContainsValueColumn = InOwnerTableView->GetHeaderRow()->IsColumnGenerated("Value");
		
		//Args.Style(&NewStyle);

		SMultiColumnTableRow<TSharedPtr<FConsoleCommand> >::Construct(Args, InOwnerTableView);

		//if (Id % 2 == 0)
		//{
		//	SetColorAndOpacity(FLinearColor(1.f, 0, 0, 1.));
		//	//SetBorderBackgroundColor(FLinearColor(255, 0, 0));
		//}
		//else
		//{
		//	SetColorAndOpacity(FLinearColor(0.f, 1.0f, 0, 1.));

		//	//SetBorderBackgroundColor(FLinearColor(0,255, 0));
		//}


		OnCanAcceptDrop = FOnCanAcceptDrop::CreateLambda(
			[=](const FDragDropEvent& DragDrop, EItemDropZone Zone, TSharedPtr<FConsoleCommand> Item)
			{
				TOptional<EItemDropZone> DropZone(Zone);
				
				return DropZone;
			}
		);
    }

    /**
     * Generate a widget for the column name.
     * Will try to convert a property to a string and return STextBlock within an SBox.
     * Override if you want to special case some columns or overhaul the widgets returned.
     */
    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	void SetFocusToCurrentValue() {
		FSlateApplication& SlateApp = FSlateApplication::Get();

		SlateApp.SetUserFocus(0, CurrentValueEditText, EFocusCause::SetDirectly);
	};

	int32 GetIndexInList() { return IndexInList; };

protected:
	FOnExecuteCommand OnExecuteCommand;

	FOnSimpleCommand OnEngineValueEditing;
	FOnCommandValueEdit OnCommandValueEdit;

    TSharedPtr<FConsoleCommand> Item;
	bool bIsValid;
	bool bIsEditable;
	bool bDisplayIcons;

	bool bContainsValueColumn = true;

	TSharedPtr<SEditableText> CurrentValueEditText;

};








class SScrollBox;

class CONSOLEMANAGER_API SConsoleManagerSlateWidget : public SDockTab
{
public:
	SLATE_BEGIN_ARGS(SConsoleManagerSlateWidget)
		: _CommandsManager()
		, _DisplayCommandValueType()
		, _DisplaySetByValue()
		, _DisplayCommandType()
	{}

	SLATE_ARGUMENT(TSharedPtr<FCommandsManager>, CommandsManager);
	SLATE_ARGUMENT(bool, DisplayCommandValueType);
	SLATE_ARGUMENT(bool, DisplaySetByValue);
	SLATE_ARGUMENT(bool, DisplayCommandType);


	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	~SConsoleManagerSlateWidget();

	void RefreshListView() { 
		if (bNeedsRefresh)
		{		
			if (CommandsManager->GetCurrentCommandGroup().Type != EGroupType::History)
			{
				CommandsListView->RebuildList();
			}
			else
			{
				GenerateCommandsScrollBox();
				CommandsListView->ScrollToBottom();
			}
		}
	};

	/**
	 * @brief Hack to set focus on next frame after construction
	*/
	void TabActivated();

	void UpdateHeaderColumnsVisibility(bool bShouldDisplayCommandValueType, bool bShouldDisplaySetByValue, bool bShouldDisplayCommandType);


	void RefreshEverything() { GenerateGroupsScrollBox(); GenerateCommandsScrollBox(); UE_LOG(LogTemp, Warning, TEXT("Data refereshed!!!")); }
	void GroupsRefresh() { GenerateGroupsScrollBox(); };
	void CommandsRefresh() { GenerateCommandsScrollBox(); };

protected:
	//virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	//virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

private:

	// 
	/** Dirty hack to set focus when everything is constructed. We could do it in tick but its better to do it in timer (fire and forget) */
	EActiveTimerReturnType SetFocusPostConstruct(double InCurrentTime, float InDeltaTime);


	bool bShowExec = true;
	bool bShowCVar = true;
	bool bShowCCmd = true;
	bool bShowOnlyModified = false;

	TSharedPtr<SCheckBox> ShowVarsCb;
	TSharedPtr<SCheckBox> ShowCmdsCb;
	TSharedPtr<SCheckBox> ShowExecsCb;

	bool bNeedsRefresh = true;

	TSharedPtr<FCommandsManager> CommandsManager;

	TSharedPtr<SScrollBox> GroupsScrollBox;
	TSharedPtr<SScrollBox> CommandsScrollBox;

	TSharedPtr< SListView< TSharedPtr<FConsoleCommand >> > CommandsListView;

	TArray<TSharedPtr<FConsoleCommand>> FilteredListView;

	FString FilterString;
	void FilterList();

	TSharedPtr<SSearchBox> SearchBox;

	void OnAddGroupButtonClicked();

	void OnSelectGroupClicked(ECheckBoxState NewRadioState, FGuid Id);

	TSharedRef< ITableRow > OnCommandsRowGenerate(TSharedPtr<FConsoleCommand> Item, const TSharedRef< STableViewBase >& OwnerTable);

	void GenerateGroupsScrollBox();

	void GenerateCommandsScrollBox();

	void RemoveGroup(FGuid Id);
	void DuplicateGroup(FGuid Id);
	void EditGroup(FGuid Id);

	bool OpenExecMultipleDialog(TArray<TSharedPtr<FConsoleCommand>> Commands);
	bool DisplayExecuteWarning(const FText& Text);

	bool DisplayTextDialog(const FText& Title, const FText& Desc, FString& InOutContent);

	TSharedPtr<SWidget> GetListViewContextMenu();

	bool HandleNewGroup(FString& OutName, UCommandsContainer*& OutContainer, UCommandsContainer* InContainer = nullptr);
	void HandleNewCommands();

	SHeaderRow::FColumn::FArguments HeaderValue = SHeaderRow::FColumn::FArguments();
	SHeaderRow::FColumn::FArguments HeaderExec = SHeaderRow::FColumn::FArguments();

	TSharedRef<SButton> GetMenuButton(FText Text, const FSlateBrush* ImageBrush, FOnClicked ClickedDelegate, TAttribute<bool> IsEnabledAttribute = true, FText TooltipText = FText());

	ECheckBoxState GetCurrentSelectedGroup(FGuid Id) const;

	int64 RowId = 0;

	bool bDisplayCommandValueType = false;
	bool bDisplaySetByValue = false;
	bool bDisplayCommandType = false;


	TMap<FString, bool> ContainersExpanded;

};
