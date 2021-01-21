// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConsoleManagerSettings.generated.h"
/**
 * 
 */
UCLASS(config = ConsoleManager, defaultconfig)
class CONSOLEMANAGER_API UConsoleManagerSettings : public UObject {
	GENERATED_BODY()

public:
	UConsoleManagerSettings(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditAnywhere, config, Category = UI)
		bool DisplayCommandValueType = true;

	UPROPERTY(EditAnywhere, config, Category = UI)
		bool DisplaySetByValue = true;

};



