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



class DragNDrop : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(DragNDrop, FDragDropOperation)

	int32 Id = 0;
	TSharedPtr<FConsoleCommand> Command;
	TSharedPtr<FCommandsManager> Manager;
};


class SConsoleCommandListRow : public SMultiColumnTableRow<TSharedPtr<FConsoleCommand>>
{
public:

    SLATE_BEGIN_ARGS(SConsoleCommandListRow) {}
		SLATE_ARGUMENT(TSharedPtr<FConsoleCommand>, Item)
		SLATE_ARGUMENT(bool, bIsValid)
	//	SLATE_ARGUMENT(int32, Id)
    SLATE_END_ARGS()


public:

	void SetOnDragDetected(FOnDragDetected Delegate)
	{
		OnDragDetectedDelegate = Delegate;
	}

	void SetOnDrop(FOnDrop Delegate)
	{
		OnDropDelegate = Delegate;
	}

	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (OnDropDelegate.IsBound())
		{
			return OnDropDelegate.Execute(MyGeometry, DragDropEvent);
		}
		return FReply::Unhandled();
	}

	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		TSharedPtr<DragNDrop> DragConnectionOp = DragDropEvent.GetOperationAs<DragNDrop>();

		if (DragConnectionOp.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Dragged obj: %s with ID: %d ||| Over: %s"), *DragConnectionOp->Command->Name, DragConnectionOp->Id, *Item->Name);
		}

		
		
		//SetOnMouseButtonDown

		UE_LOG(LogTemp, Warning, TEXT(""));
		
	}

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (OnDragDetectedDelegate.IsBound())
		{
			return OnDragDetectedDelegate.Execute(MyGeometry, MouseEvent);
		}

		return FReply::Unhandled();


		//UE_LOG(LogTemp, Warning, TEXT("Drag DETECTED"));

		//TSharedRef<DragNDrop> DragOp = MakeShareable(new DragNDrop);

		////DragOp->Manager = 


		//return FReply::Handled().BeginDragDrop(DragOp);
	}

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
    {
        Item = InArgs._Item;
		bIsValid = InArgs._bIsValid;
		//Id = InArgs._Id;

		//SNew(SBorder)
		//	.BorderBackgroundColor(bIsValid ? FSlateColor(FLinearColor(255, 0, 0)) : FSlateColor(FLinearColor(255, 255, 255)))
		//	[

		//	];
		SMultiColumnTableRow<TSharedPtr<FConsoleCommand> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);

    }

    /**
     * Generate a widget for the column name.
     * Will try to convert a property to a string and return STextBlock within an SBox.
     * Override if you want to special case some columns or overhaul the widgets returned.
     */
    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

protected:
    TSharedPtr<FConsoleCommand> Item;
	bool bIsValid;
	int32 Id;

	FOnDrop OnDropDelegate;
	FOnDragDetected OnDragDetectedDelegate;
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

	void RefreshListView() { CommandsListView->RebuildList(); };

protected:
	//virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;


private:

	TWeakPtr<FCommandsManager> CommandsManager;

	TSharedPtr<SScrollBox> GroupsScrollBox;
	TSharedPtr<SScrollBox> CommandsScrollBox;

	TSharedPtr< SListView< TSharedPtr<FConsoleCommand >> > CommandsListView;

	TArray<TSharedPtr<FConsoleCommand>> FilteredListView;


	TSharedPtr<SSearchBox> SearchBox;

	void OnAddGroupButtonClicked();

	FReply OnSelectGroupClicked(int Id);
	FReply OnSelectCommandClicked(int Id);

	TSharedRef< ITableRow > OnCommandsRowGenerate(TSharedPtr<FConsoleCommand> Item, const TSharedRef< STableViewBase >& OwnerTable);

	void GenerateGroupsScrollBox();

	void GenerateCommandsScrollBox();

	void RemoveGroup(int Id);
	void DuplicateGroup(int Id);
	void EditGroup(int Id);

	bool OpenExecMultipleDialog(TArray<TSharedPtr<FConsoleCommand>> Commands);


	TSharedPtr<SWidget> GetListViewContextMenu();

	FCommandGroup* HandleNewGroup();

	bool bIsAllCommands = false;

};
