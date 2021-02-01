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

	/** Contains information what type of variable to use (only applies to CVar's) */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show variable type"))
		bool DisplayCommandValueType = true;

	/** Shows by whom variable was set */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show Set By"))
		bool DisplaySetByValue = true;

	/** Contains information of type(CVar, CCmd, Exec) */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show command type"))
		bool DisplayCommandType = false;

	/** The color used to visualize matching values */
	UPROPERTY(EditAnywhere, config, Category = Colors, meta = (DisplayName = "Matching values color"))
		FLinearColor MatchingValuesColor;

	/** The color used to visualize difference between values */
	UPROPERTY(EditAnywhere, config, Category = Colors, meta = (DisplayName = "Different values color"))
		FLinearColor NotMatchingValuesColor;

};



