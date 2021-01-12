// Fill out your copyright notice in the Description page of Project Settings.


#include "ConsoleManagerCommands.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerCommands::RegisterCommands()
{
	UI_COMMAND(OpenTab, "ConsoleManager", "Open Console Manager Tab", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
