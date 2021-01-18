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
	TArray<FConsoleCommand> Commands;
	bool bInitiallySet = false;
};

