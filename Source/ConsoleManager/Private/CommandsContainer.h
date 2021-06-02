// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CommandStructs.h"
#include "CommandsContainer.generated.h"


DECLARE_DELEGATE_OneParam(FDestroyCalled, UCommandsContainer*);
DECLARE_DELEGATE_OneParam(FRenamed, UCommandsContainer*);
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


	FDestroyCalled OnDestroyCalled;
	FRenamed OnRenamed;

	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChanged) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChainChanged) override;
	virtual void PostEditImport() override;

	virtual void PostRename(UObject* OldOuter, const FName OldName) override;

	virtual void BeginDestroy() override;

	FCommandGroup* GetGroupByName(const FString& Name);
};
