// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandsContainer.h"


UCommandsContainer::UCommandsContainer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//UE_LOG(LogTemp, Warning, TEXT("Group count: %d"), Groups.Num());

	//for (auto& Group : Groups)
	//{
	//	for (auto& Command : Group.Commands)
	//	{
	//		Command.InitializeLoaded();
	//	}
	//}
}

void UCommandsContainer::PostEditChangeProperty(FPropertyChangedChainEvent& Event)
{
	UE_LOG(LogTemp, Warning, TEXT("Property changed!"));

	
}
