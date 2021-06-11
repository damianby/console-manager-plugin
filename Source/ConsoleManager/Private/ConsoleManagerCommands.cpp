// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "ConsoleManagerCommands.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

void FConsoleManagerCommands::RegisterCommands()
{
	GroupContextMenu = MakeShareable(new FUICommandList);

	UI_COMMAND(OpenTab, "Open all containers", "Opens Console Manager with all found containers", EUserInterfaceActionType::Button, FInputGesture());

	UI_COMMAND(OpenTabLast, "Open last selected", "Opens Console Manager with previously selected containers", EUserInterfaceActionType::Button, FInputGesture());

	
}

#undef LOCTEXT_NAMESPACE
