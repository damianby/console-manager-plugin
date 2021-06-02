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
	UE_LOG(LogTemp, Warning, TEXT("Real constructor for object: %s"), *GetName());
}

void UCommandsContainer::PostInitProperties()
{
	Super::PostInitProperties();

	UE_LOG(LogTemp, Warning, TEXT("PostInitProperties | Group count: %d   | Name: %s"), Groups.Num(), *GetName());
}

void UCommandsContainer::PostLoad()
{
	Super::PostLoad();
	UE_LOG(LogTemp, Warning, TEXT("POSTLOAD! %s"), *GetName());

	UE_LOG(LogTemp, Warning, TEXT("Group count: %d"), Groups.Num());

	for (auto& Group : Groups)
	{
		for (auto& Command : Group.Commands)
		{
			Command.InitializeLoaded();
			UE_LOG(LogTemp, Warning, TEXT("DATA POSTLOAD : %s : %s : %s : %s"), *Command.GetName(), *Command.GetValue(), *Command.GetExec(), *Command.GetSetBy());
		}
	}
}

void UCommandsContainer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChanged)
{
	Super::PostEditChangeProperty(PropertyChanged);
	UE_LOG(LogTemp, Warning, TEXT("Post edit: count %d"), Groups.Num());

}

void UCommandsContainer::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChainChanged)
{
	Super::PostEditChangeChainProperty(PropertyChainChanged);
	UE_LOG(LogTemp, Warning, TEXT("postedit chain: count %d"), Groups.Num());

}

void UCommandsContainer::PostEditImport()
{
	Super::PostEditImport();
	UE_LOG(LogTemp, Warning, TEXT("Post edit import: count %d"), Groups.Num());
}

void UCommandsContainer::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
	OnRenamed.ExecuteIfBound(this);
	UE_LOG(LogTemp, Warning, TEXT("Container renamed"));
}

void UCommandsContainer::BeginDestroy()
{
	OnDestroyCalled.ExecuteIfBound(this);
	UE_LOG(LogTemp, Warning, TEXT("Container being destroyed!"));
	Super::BeginDestroy();
}

FCommandGroup* UCommandsContainer::GetGroupByName(const FString& Name)
{
	for (FCommandGroup& Group : Groups)
	{
		if (Group.Name.Equals(Name))
		{
			return &Group;
		}
	}
	return nullptr;
}
	
