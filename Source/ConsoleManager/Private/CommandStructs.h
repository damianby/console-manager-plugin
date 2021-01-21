#pragma once

#include "CoreMinimal.h"

enum class EConsoleCommandInputType : uint8
{
	None,
	Value,
	Slider
};


class FConsoleCommand
{
public:
	FConsoleCommand(FString _Command);

	FString Name;
	EConsoleCommandInputType InputType;
	FString Command;
	FString Value;
	FString CurrentValue;
	FString Type;
	FString SetBy;

	bool IsValid = true;

	FORCEINLINE bool operator == (const FString& Other) const {
		return Name.Equals(Other);
	};

	void Refresh();
	FString GetTooltip();
	
private:

	static FString GetTextSection(const TCHAR * &It);

	bool bIsInitiallySet = false;

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

