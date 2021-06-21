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

UENUM(BlueprintType)
enum class EConsoleCommandVarType : uint8
{
	None UMETA(DisplayName = "None"),
	Int UMETA(DisplayName = "Int"),
	Float UMETA(DisplayName = "Float"),
	Bool UMETA(DisplayName = "Bool"),
	String UMETA(DisplayName = "String")
};

UENUM(BlueprintType)
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
	FConsoleCommand() {
		UE_LOG(LogTemp, Warning, TEXT("DEFAULT CONSTRUCTOR is %s : %s : %s"), *Name, *Value, *ExecCommand);
	};

	FConsoleCommand(FString _Command, bool bOnlyName = false);
	FConsoleCommand(const FConsoleCommand& Copy);

	void SetIsValid(bool NewValid) { bIsValid = NewValid; };
	bool IsValid() { return bIsValid; };
	FORCEINLINE bool operator == (const FString& Other) const {
		return Name.Equals(Other, ESearchCase::IgnoreCase);
	};

	FORCEINLINE bool operator < (const FConsoleCommand& Other) const {
		return Name < Other.Name;
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

	FORCEINLINE void SetNote(const FString& NewNote){ Note = NewNote; }
	const FString& GetValue() const { return Value; };

	FORCEINLINE const FString& GetName() const { return Name; }
	FORCEINLINE const FString& GetSetBy() const { return SetBy; }
	FORCEINLINE const EConsoleCommandVarType& GetType() const { return Type; }
	FORCEINLINE const EConsoleCommandType& GetObjType() const { return ObjType; }
	FORCEINLINE const FString& GetNote() const { return Note; }
	FORCEINLINE bool IsReadOnly() const { return bReadOnly; }
	FORCEINLINE bool IsNameAsValue() const { return bNameAsValue; }

	void SetOnlyName(bool Val) { bNameAsValue = Val; }

	const FString& GetCurrentValue() const { return CurrentValue; };

	void Refresh();
	FString GetTooltip();
	
	// We expose only these properties because there is no need to save rest, they are updated when needed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
		EConsoleCommandType ObjType;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
		FString Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
		FString Value;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
		FString Note;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
		EConsoleCommandVarType Type = EConsoleCommandVarType::None;
	
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString ExecCommand;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString CurrentValue;
	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString SetBy;

	void InitializeLoaded();

private:
	FORCEINLINE void RefreshExec() { ExecCommand = Name + " " + Value; };

	void InitialParse(IConsoleObject* Obj);

	static FString GetTextSection(const TCHAR * &It);

	bool bIsValid = true;
	bool bIsInitiallyParsed = false;

	bool bNameAsValue = false;
	bool bReadOnly = false;

	IConsoleObject* CachedObj = nullptr;
};

USTRUCT(BlueprintType)
struct FCommandGroup
{
	GENERATED_BODY()
public:

	FCommandGroup() { };
	//FCommandGroup(const FCommandGroup& Copy);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FString Name;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	FGuid Id;

	EGroupType Type = EGroupType::Default;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Groups")
	TArray<FConsoleCommand> Commands;

	bool bInitiallySet = false;
	bool bIsEditable = true;

	FORCEINLINE bool operator == (const FGuid& Other) const {
		return Id == Other;
	};
};

