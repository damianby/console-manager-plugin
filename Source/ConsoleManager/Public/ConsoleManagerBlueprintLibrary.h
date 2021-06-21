// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ConsoleManagerBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CONSOLEMANAGER_API UConsoleManagerBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	



	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Group", Keywords = "console manager execute"), Category = "Console Manager")
	static bool ExecuteGroup(UCommandsContainer* Container, const FString& Name);

};
