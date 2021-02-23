// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CommandsContainerFactoryNew.generated.h"

/**
 * 
 */
UCLASS(hidecategories = Object)
class CONSOLEMANAGER_API UCommandsContainerFactoryNew : public UFactory
{
	GENERATED_BODY()

public:
	
	UCommandsContainerFactoryNew(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
	
};
