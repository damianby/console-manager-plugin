// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ConsoleManagerStyle.h"

class FConsoleManagerCommands : public TCommands<FConsoleManagerCommands>
{
public:

	FConsoleManagerCommands()
		: TCommands<FConsoleManagerCommands>(TEXT("ConsoleManager"), NSLOCTEXT("Contexts", "ConsoleManager", "Console Manager Plugin"), NAME_None, FConsoleManagerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenTab;

	TSharedPtr<FUICommandList> GroupContextMenu;

	TSharedPtr<FUICommandInfo> NewGroupCommand;
};
