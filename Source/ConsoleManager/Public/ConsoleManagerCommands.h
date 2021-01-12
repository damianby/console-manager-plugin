// Fill out your copyright notice in the Description page of Project Settings.

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
};
