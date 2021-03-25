// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ConsoleManagerSettings.generated.h"
/**
 * 
 */

class UCommandsContainer;


UENUM(BlueprintType)
enum class EConsoleManagerStartupOption : uint8
{
	AllContainers UMETA(DisplayName = "All containers"),
	LastOpened	UMETA(DisplayName = "Last opened"),
	Specified	UMETA(DisplayName = "Specified")
};

UCLASS(config = ConsoleManager)
class CONSOLEMANAGER_API UConsoleManagerSettings : public UObject {
	GENERATED_BODY()

public:
	UConsoleManagerSettings(const FObjectInitializer& ObjectInitializer);


	/** Global shortcut to open manager */
	UPROPERTY(EditAnywhere, config, Category = Shortcuts, meta = (DisplayName = "Open manager shortcut"))
		FInputChord OpenShortcut = FInputChord();


	/** Commands font size */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Commands font size"))
		int32 CommandsFontSize = 9;

	/** Should toolbar button be visible next to settings (when disabled Console Manager will be accessible from Window tab)  */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show toolbar button"))
		bool bShowToolbar = true;

	UPROPERTY(config, meta = (DisplayName = "Last Selected Objects"))
		TArray< TSoftObjectPtr<UCommandsContainer>> LastSelectedObjs;

	/** Contains information what type of variable to use (only applies to CVar's) */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show variable type"))
		bool DisplayCommandValueType = true;

	/** Shows by whom variable was set */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show Set By"))
		bool DisplaySetByValue = true;

	/** Contains information of type(CVar, CCmd, Exec) */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "Show command type"))
		bool DisplayCommandType = false;

	/** How many commands can history keep */
	UPROPERTY(EditAnywhere, config, Category = UI, meta = (DisplayName = "History buffer size"))
		int32 HistoryBufferSize = 64;

	/** The color used to visualize matching values */
	UPROPERTY(EditAnywhere, config, Category = Colors, meta = (DisplayName = "Matching values color"))
		FLinearColor MatchingValuesColor;

	/** The color used to visualize difference between values */
	UPROPERTY(EditAnywhere, config, Category = Colors, meta = (DisplayName = "Different values color"))
		FLinearColor NotMatchingValuesColor;

	/** Which commands to open when clicking on toolbar button or Window menu */
	UPROPERTY(EditAnywhere, config, Category = Startup, meta = (DisplayName = "Default container on open"))
		EConsoleManagerStartupOption StartupOption = EConsoleManagerStartupOption::AllContainers;

	UPROPERTY(EditAnywhere, config, Category = Startup, meta = (DisplayName = "Asset to load", EditCondition = "StartupOption==EConsoleManagerStartupOption::Specified"))
		TSoftObjectPtr<UCommandsContainer> AssetToLoad;

};



