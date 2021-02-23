// Fill out your copyright notice in the Description page of Project Settings.

#include "CommandsContainerFactoryNew.h"

#include "CommandsContainer.h"



/* UCommandsContainerFactoryNew structors
 *****************************************************************************/

UCommandsContainerFactoryNew::UCommandsContainerFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UCommandsContainer::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UCommandsContainerFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCommandsContainer>(InParent, InClass, InName, Flags);
}

bool UCommandsContainerFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
