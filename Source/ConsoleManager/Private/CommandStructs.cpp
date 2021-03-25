// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "CommandStructs.h"

static inline bool IsWhiteSpace(TCHAR Value) { return Value == TCHAR(' '); }

static const TCHAR* GetSetByTCHAR(EConsoleVariableFlags InSetBy)
{
	EConsoleVariableFlags SetBy = (EConsoleVariableFlags)((uint32)InSetBy & ECVF_SetByMask);

	switch (SetBy)
	{
#define CASE(A) case ECVF_SetBy##A: return TEXT(#A);
		// Could also be done with enum reflection instead
		CASE(Constructor)
			CASE(Scalability)
			CASE(GameSetting)
			CASE(ProjectSetting)
			CASE(DeviceProfile)
			CASE(SystemSettingsIni)
			CASE(ConsoleVariablesIni)
			CASE(Commandline)
			CASE(Code)
			CASE(Console)
#undef CASE
	}
	return TEXT("<UNKNOWN>");
}

FConsoleCommand::FConsoleCommand(FString _Command)
{
	const TCHAR* It = *_Command;

	FString Param1 = GetTextSection(It);
	if (Param1.IsEmpty())
	{
		bIsValid = false;
	}

	Name = Param1;
	Value = _Command.Mid(Param1.Len() + 1);

	FString FoundName;
	IConsoleObject* Obj = nullptr;

	IConsoleManager::Get().ForEachConsoleObjectThatContains(FConsoleObjectVisitor::CreateLambda(
		[&Param1, &FoundName, &Obj](const TCHAR* CurrentObjName, IConsoleObject* CurrentObj) {
			if (Param1.Equals(CurrentObjName, ESearchCase::IgnoreCase))
			{
				FoundName = FString(CurrentObjName);	
				Obj = CurrentObj;
			}
		}),
		*Name);
	
	CachedObj = Obj;

	if (!FoundName.IsEmpty())
	{
		Name = FoundName;
	}

	InitialParse(CachedObj);
	RefreshExec();
}

FConsoleCommand::FConsoleCommand(const FConsoleCommand& Copy)
{
	// When we copy we do it only for these values as rest is dynamically loaded
	Name = Copy.Name;
	Value = Copy.Value;
	RefreshExec();



	//CurrentValue = Copy.CurrentValue;
	ObjType = Copy.ObjType;
	Type = Copy.Type;
	bIsValid = Copy.bIsValid;
	bIsInitiallyParsed = Copy.bIsInitiallyParsed;
	Note = Copy.Note;
	SetBy = Copy.SetBy;

	
	UE_LOG(LogTemp, Warning, TEXT("Data is %s : %s : %s"), *Name, *Value, *ExecCommand);
	//}
	//else
	//{
	//	FConsoleCommand(Copy.ExecCommand);
	//}
}


void FConsoleCommand::Refresh()
{
	if (!CachedObj)
	{
		CachedObj = IConsoleManager::Get().FindConsoleObject(*Name);
	}

	if (CachedObj)
	{
		SetBy = GetSetByTCHAR(CachedObj->GetFlags());

		IConsoleVariable* CVar = CachedObj->AsVariable();
		if (CVar)
		{
			CurrentValue = CVar->GetString();
		}
	}

}
void FConsoleCommand::InitializeLoaded()
{
	RefreshExec();
}
void FConsoleCommand::InitialParse(IConsoleObject* Obj)
{
	if (Obj)
	{
		SetBy = GetSetByTCHAR(Obj->GetFlags());

		IConsoleCommand* CCmd = Obj->AsCommand();
		IConsoleVariable* CVar = Obj->AsVariable();
		
		if (CVar)
		{
			CurrentValue = CVar->GetString();
			ObjType = EConsoleCommandType::CVar;

			if (CVar->IsVariableBool())
			{
				Type = EConsoleCommandVarType::Bool;
			}
			else if (CVar->IsVariableFloat())
			{
				Type = EConsoleCommandVarType::Float;
			}
			else if (CVar->IsVariableInt())
			{
				Type = EConsoleCommandVarType::Int;
			}
			else if (CVar->IsVariableString())
			{
				Type = EConsoleCommandVarType::String;
			}
		}
		else if (CCmd)
		{
			ObjType = EConsoleCommandType::CCmd;
		}

		if (Obj->TestFlags(ECVF_Unregistered))
		{
			//Handle unregistered!
			UE_LOG(LogTemp, Warning, TEXT("No test flag"));

			bIsValid = false;
		}
		else
		{
			bIsValid = true;
		}
	}
	else
	{
		//Its engine command?
		ObjType = EConsoleCommandType::Exec;
	}

	bIsInitiallyParsed = true;
}

FString FConsoleCommand::GetTooltip()
{
	FString HelpString;

	IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Name);
	if (Obj)
	{
		IConsoleVariable* CVar = Obj->AsVariable();
		IConsoleCommand* CCmd = Obj->AsCommand();

		if (CCmd)
		{
			HelpString = "Console Command:" LINE_TERMINATOR + ExecCommand + LINE_TERMINATOR LINE_TERMINATOR;
		}
		else if (CVar)
		{
			HelpString = "Console Variable:" LINE_TERMINATOR + ExecCommand + LINE_TERMINATOR LINE_TERMINATOR;
		}

		HelpString += Obj->GetHelp();
	}
	else
	{
		HelpString += "Exec commands have no help";
	}

	if (!Note.IsEmpty())
	{
		HelpString += LINE_TERMINATOR LINE_TERMINATOR "Note:" LINE_TERMINATOR + Note;
	}

	return HelpString;
}

FString FConsoleCommand::GetTextSection(const TCHAR*& It)
{
	FString ret;

	while (*It)
	{
		if (IsWhiteSpace(*It))
		{
			break;
		}

		ret += *It++;
	}

	while (IsWhiteSpace(*It))
	{
		++It;
	}

	return ret;
}
//
//FCommandGroup::FCommandGroup(const FCommandGroup& Copy)
//{
//	Name = Copy.Name;
//	Id = Copy.Id;
//	Type = Copy.Type;
//
//	ContainerSoftPtr = Copy.ContainerSoftPtr;
//
//	Commands = Copy.Commands;
//
//	bInitiallySet = Copy.bInitiallySet;
//	bIsEditable = Copy.bIsEditable;
//}
