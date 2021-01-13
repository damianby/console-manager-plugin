#pragma once

#include "CoreMinimal.h"

struct FCommandGroup
{
	FString Name;
	TArray<FString> Commands;
};

enum class EConsoleCommandInputType : uint8
{
	None,
	Value,
	Slider
};

struct FConsoleCommand
{
	FString Name;
	EConsoleCommandInputType InputType;
	bool IsValid;
	FString Command;
	FString Value;
	FString Type;
	FString SetBy;
};