#pragma once

#include "CoreMinimal.h"

enum class EConsoleCommandInputType : uint8
{
	None,
	Value,
	Slider
};


enum class EConsoleCommandType : uint8
{
	CVar,
	CCmd
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

	EConsoleCommandType ObjType;

	bool IsValid = true;

	FORCEINLINE bool operator == (const FString& Other) const {
		return Name.Equals(Other);
	};

	//FORCEINLINE const FString& GetExec() { return ExecCommand; };

	void Refresh();
	FString GetTooltip();
	
private:

	static FString GetTextSection(const TCHAR * &It);

	bool bIsInitiallySet = false;

	//FString ExecCommand;

};

struct FCommandGroup
{
	FString Name;
	FString Id;
	TArray<FConsoleCommand> Commands;
	bool bInitiallySet = false;
	bool bIsEditable = true;

	FORCEINLINE bool operator == (const FString& Other) const {
		return Id.Equals(Other);
	};
};

