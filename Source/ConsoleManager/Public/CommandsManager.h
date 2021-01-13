// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FileHelper.h"
#include "Interfaces/IPluginManager.h"
#include "CommandStructs.h"

/**
 * 
 */
class CONSOLEMANAGER_API FCommandsManager
{
public:

	FCommandsManager();

	bool Initialize(const FString& Path);

	void Refresh();

	const TArray<FConsoleCommand>& GetCurrentCommands();
	const TArray<FString> GetGroupList();

private:

	FCommandGroup ConsoleHistory;

	TArray<FCommandGroup> CommandGroups;

	TArray<FConsoleCommand> CurrentCommands;
};
