// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"

#include "CommandStructs.generated.h"


enum class EGroupType : uint8
{
	Default,
	AllCommands,
	History
};

enum class EConsoleCommandType : uint8
{
	CVar,
	CCmd,
	Exec
};

USTRUCT(BlueprintType)
struct FConsoleCommand
{
	GENERATED_BODY()

public:
	FConsoleCommand() {};

	FConsoleCommand(FString _Command);
	FConsoleCommand(const FConsoleCommand& Copy);

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
	const FString& GetValue() const { return Value; };

	FORCEINLINE const FString& GetName() const { return Name; }
	FORCEINLINE const FString& GetSetBy() const { return SetBy; }
	FORCEINLINE const FString& GetType() const { return Type; }
	FORCEINLINE const EConsoleCommandType& GetObjType() const { return ObjType; }


	const FString& GetCurrentValue() { return CurrentValue; };

	void Refresh();
	FString GetTooltip();
	
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString ExecCommand;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString CurrentValue;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Value;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString SetBy;

	// These values are initialized on object creation and not changed later 
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Name;
	
	EConsoleCommandType ObjType;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Type;
	//



private:

	FORCEINLINE void RefreshExec() { ExecCommand = Name + " " + Value; };

	void InitialParse(IConsoleObject* Obj);

	

	static FString GetTextSection(const TCHAR * &It);

	bool bIsValid = true;
	bool bIsInitiallyParsed = false;
};

USTRUCT(BlueprintType)
struct FCommandGroup
{
	GENERATED_BODY()
public:

	FCommandGroup() { };

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Id;

	EGroupType Type;

	FSoftObjectPtr ContainerSoftPtr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	TArray<FConsoleCommand> Commands;

	bool bInitiallySet = false;
	bool bIsEditable = true;

	FORCEINLINE bool operator == (const FString& Other) const {
		return Id.Equals(Other);
	};
};

