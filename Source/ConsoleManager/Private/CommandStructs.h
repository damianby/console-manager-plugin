#pragma once

#include "CoreMinimal.h"

enum class EConsoleCommandInputType : uint8
{
	None,
	Value,
	Slider
};



struct FConsoleCommand
{
	FConsoleCommand(FString _Command) : Command(_Command) {}

	FString Name;
	EConsoleCommandInputType InputType;
	bool IsValid;
	FString Command;
	FString Value;
	FString Type;
	FString SetBy;
};

struct FCommandGroup
{
	FString Name;
	FString Id;
	TArray<FConsoleCommand> Commands;
	bool bInitiallySet = false;

	FORCEINLINE bool operator == (const FString& Other) const {
		return Id.Equals(Other);
	};
};

