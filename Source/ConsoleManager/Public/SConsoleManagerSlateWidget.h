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



class SConsoleCommandListRow : public SMultiColumnTableRow<TSharedPtr<FConsoleCommand>>
{
public:

    SLATE_BEGIN_ARGS(SConsoleCommandListRow) {}
	SLATE_ARGUMENT(TSharedPtr<FConsoleCommand>, Item)
		SLATE_ARGUMENT(bool, bIsValid)
        SLATE_END_ARGS()

public:

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
    {
        Item = InArgs._Item;
		bIsValid = InArgs._bIsValid;

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

	bool bIsAllCommands = false;

};
