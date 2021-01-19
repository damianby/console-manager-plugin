// Fill out your copyright notice in the Description page of Project Settings.


#include "ConsoleManagerCommands.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerCommands::RegisterCommands()
{
	GroupContextMenu = MakeShareable(new FUICommandList);

	UI_COMMAND(OpenTab, "ConsoleManager", "Open Console Manager Tab", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(NewGroupCommand, "NewGroupCommand", "Create New Group", EUserInterfaceActionType::Button, FInputGesture());

}

#undef LOCTEXT_NAMESPACE
