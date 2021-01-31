// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
#include "CommandsManager.h"
#include "SGroupButton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"

/**
 * 
 */

#include <chrono>



class DragNDrop : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(DragNDrop, FDragDropOperation)

	int32 Id = 0;
	TSharedPtr<FConsoleCommand> Command;
	TSharedPtr<FCommandsManager> Manager;
};


DECLARE_DELEGATE_RetVal_OneParam(bool, FOnExecuteCommand, TSharedPtr<FConsoleCommand>)
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
	//	SLATE_ARGUMENT(int32, Id)
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
		
		//static FTableRowStyle NewStyle = FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");
		//NewStyle.DropIndicator_Onto = NewStyle.;

		FTableRowArgs Args;
		Args.Padding(FMargin(0, 2.0f));
		
		bContainsValueColumn = InOwnerTableView->GetHeaderRow()->IsColumnGenerated("Value");
		
		//Args.Style(&NewStyle);

		SMultiColumnTableRow<TSharedPtr<FConsoleCommand> >::Construct(Args, InOwnerTableView);

		//SetBorderBackgroundColor(FLinearColor(255, 0, 0));

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

protected:
	FOnExecuteCommand OnExecuteCommand;

	FOnSimpleCommand OnEngineValueEditing;
	FOnCommandValueEdit OnCommandValueEdit;

    TSharedPtr<FConsoleCommand> Item;
	bool bIsValid;
	bool bIsEditable;
	int32 Id;

	bool bContainsValueColumn = true;

};








class SScrollBox;

class CONSOLEMANAGER_API SConsoleManagerSlateWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConsoleManagerSlateWidget)
	{}

	SLATE_ARGUMENT(TWeakPtr<FCommandsManager>, CommandsManager);
	SLATE_ARGUMENT(bool, DisplayCommandValueType);
	SLATE_ARGUMENT(bool, DisplaySetByValue);


	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void RefreshListView() { 
		if (bNeedsRefresh)
		{
			//CommandsListView->RequestListRefresh();
			CommandsListView->RebuildList();
		}
	};

protected:
	//virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;


private:

	bool bNeedsRefresh = true;

	TWeakPtr<FCommandsManager> CommandsManager;

	TSharedPtr<SScrollBox> GroupsScrollBox;
	TSharedPtr<SScrollBox> CommandsScrollBox;

	TSharedPtr< SListView< TSharedPtr<FConsoleCommand >> > CommandsListView;

	TArray<TSharedPtr<FConsoleCommand>> FilteredListView;


	TSharedPtr<SSearchBox> SearchBox;

	void OnAddGroupButtonClicked();

	FReply OnSelectGroupClicked(int Id);

	TSharedRef< ITableRow > OnCommandsRowGenerate(TSharedPtr<FConsoleCommand> Item, const TSharedRef< STableViewBase >& OwnerTable);

	void GenerateGroupsScrollBox();

	void GenerateCommandsScrollBox();

	void RemoveGroup(int Id);
	void DuplicateGroup(int Id);
	void EditGroup(int Id);

	bool OpenExecMultipleDialog(TArray<TSharedPtr<FConsoleCommand>> Commands);


	TSharedPtr<SWidget> GetListViewContextMenu();

	FCommandGroup* HandleNewGroup();
	void HandleNewCommands();

	bool bIsAllCommands = false;

	SHeaderRow::FColumn::FArguments HeaderValue = SHeaderRow::FColumn::FArguments();

};
