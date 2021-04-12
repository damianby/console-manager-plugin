// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CommandStructs.h"
#include "CommandsContainer.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, hidecategories=(Object))
class CONSOLEMANAGER_API UCommandsContainer : public UObject
{
	GENERATED_BODY()

public:

	UCommandsContainer(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	TArray<FCommandGroup> Groups;


	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChanged) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChainChanged) override;
	virtual void PostEditImport() override;

	FCommandGroup* GetGroupByName(const FString& Name);
};
