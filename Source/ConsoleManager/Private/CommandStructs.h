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
	CCmd,
	Exec
};

class FConsoleCommand
{
public:
	FConsoleCommand(FString _Command);
	FConsoleCommand(const FConsoleCommand& Copy);

	FString Name;
	EConsoleCommandInputType InputType;
	FString Value;

	FString Type;
	FString SetBy;

	EConsoleCommandType ObjType;



	void SetIsValid(bool NewValid) { bIsValid = NewValid; };
	bool IsValid() { return bIsValid; };
	FORCEINLINE bool operator == (const FString& Other) const {
		return Name.Equals(Other, ESearchCase::IgnoreCase);
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

	void InitialParse(IConsoleObject* Obj);

	FString ExecCommand;
	FString CurrentValue;

	static FString GetTextSection(const TCHAR * &It);

	bool bIsValid = true;
	bool bIsInitiallyParsed = false;
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

