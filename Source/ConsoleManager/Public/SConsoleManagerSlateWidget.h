// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
#include "CommandsManager.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SScrollBox;

class CONSOLEMANAGER_API SConsoleManagerSlateWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConsoleManagerSlateWidget)
	{}

	SLATE_ARGUMENT(TWeakPtr<FCommandsManager>, CommandsManager);

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);


private:

	TWeakPtr<FCommandsManager> CommandsManager;

	TSharedPtr<SScrollBox> GroupsScrollBox;
	TSharedPtr<SScrollBox> CommandsScrollBox;

	FReply OnAddGroupButtonClicked();

	FReply OnSelectGroupClicked(int Id);
	FReply OnSelectCommandClicked(int Id);

	void GenerateGroupsScrollBox();

	void GenerateCommandsScrollBox();
};
