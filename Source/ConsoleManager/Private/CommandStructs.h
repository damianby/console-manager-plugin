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
	FString Value;

	FString Type;
	FString SetBy;

	EConsoleCommandType ObjType;



	void SetIsValid(bool NewValid) { bIsValid = NewValid; };
	bool IsValid() { return bIsValid; };
	FORCEINLINE bool operator == (const FString& Other) const {
		return Name.Equals(Other);
	};

	FORCEINLINE const FString& GetExec() const
	{ 
		return ExecCommand; 
	};

	FORCEINLINE void SetValue(const FString& NewValue)
	{
		Value = NewValue;
		RefreshExec();
	};
	const FString& GetValue() { return Value; };

	const FString& GetCurrentValue() { return CurrentValue; };

	void Refresh();
	FString GetTooltip();
	
private:

	FORCEINLINE void RefreshExec() { ExecCommand = Name + " " + Value; };

	FString ExecCommand;
	FString CurrentValue;

	static FString GetTextSection(const TCHAR * &It);

	bool bIsValid = true;
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

