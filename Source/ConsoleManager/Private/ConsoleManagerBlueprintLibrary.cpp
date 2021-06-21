// Fill out your copyright notice in the Description page of Project Settings.


#include "ConsoleManagerBlueprintLibrary.h"

#include "CommandsContainer.h"
#include "ConsoleManager.h"

bool UConsoleManagerBlueprintLibrary::ExecuteGroup(UCommandsContainer* Container, const FString& Name)
{
	FCommandsManager* Manager = FConsoleManagerModule::GetModule().GetCommandsManager();

	bool bSuccess = Manager->ExecuteGroup(Container, Name);

	return bSuccess;
}
