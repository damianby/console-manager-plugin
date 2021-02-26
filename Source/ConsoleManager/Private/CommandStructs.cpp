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
		[=, &FoundName, &Obj](const TCHAR* CurrentObjName, IConsoleObject* CurrentObj) {
			if (Param1.Equals(CurrentObjName, ESearchCase::IgnoreCase))
			{
				FoundName = FString(CurrentObjName);	
				Obj = CurrentObj;
			}
		}),
		*Name);
		
	if (!FoundName.IsEmpty())
	{
		Name = FoundName;
	}

	InitialParse(Obj);
	RefreshExec();
}

FConsoleCommand::FConsoleCommand(const FConsoleCommand& Copy)
{
	//if (bIsInitiallyParsed)
	//{
		Name = Copy.Name;
		Value = Copy.Value;
		//CurrentValue = Copy.CurrentValue;
		ObjType = Copy.ObjType;
		Type = Copy.Type;
		bIsValid = Copy.bIsValid;
		bIsInitiallyParsed = Copy.bIsInitiallyParsed;

		RefreshExec();
	//}
	//else
	//{
	//	FConsoleCommand(Copy.ExecCommand);
	//}
}


void FConsoleCommand::Refresh()
{
	IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Name);
	if (Obj)
	{
		SetBy = GetSetByTCHAR(Obj->GetFlags());

		IConsoleVariable* CVar = Obj->AsVariable();
		if (CVar)
		{
			CurrentValue = CVar->GetString();
		}
	}
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
				Type = "Bool";
			}
			else if (CVar->IsVariableFloat())
			{
				Type = "Float";
			}
			else if (CVar->IsVariableInt())
			{
				Type = "Int";
			}
			else if (CVar->IsVariableString())
			{
				Type = "String";
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
			HelpString = "Console Command:\n" + ExecCommand + "\n\n";
		}
		else if (CVar)
		{
			HelpString = "Console Variable:\n" + ExecCommand + "\n\n";
		}

		HelpString += Obj->GetHelp();
	}
	else
	{
		HelpString += "Exec commands have no help";
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
