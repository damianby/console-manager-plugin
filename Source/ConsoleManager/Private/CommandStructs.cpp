
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

	IConsoleManager::Get().ForEachConsoleObjectThatContains(FConsoleObjectVisitor::CreateLambda(
		[Param1, &FoundName](const TCHAR* CurrentObjName, IConsoleObject* CurrentObj) {
			if (Param1.Equals(CurrentObjName, ESearchCase::IgnoreCase))
			{
				FoundName = FString(CurrentObjName);
			}
		}),
		*Name);
		
	if (!FoundName.IsEmpty())
	{
		Name = FoundName;
	}

	RefreshExec();

	//bool Found = false;
	//FConsoleObjectVisitor Visitor;
	//Visitor.BindLambda([Param1, &Found](const TCHAR* Name, IConsoleObject* Obj) {

	//	if (Param1.Equals(Name))
	//	{
	//		Found = true;
	//	}
	//	});
	//

	//IConsoleManager::Get().ForEachConsoleObjectThatStartsWith(Visitor, *Name);
	//UE_LOG(LogTemp, Warning, TEXT("Found command %s with %d"), *Name, Found);

}

FConsoleCommand::FConsoleCommand(const FConsoleCommand& Copy)
{
	Name = Copy.Name;
	Value = Copy.Value;
	RefreshExec();
}


void FConsoleCommand::Refresh()
{
	if (bIsInitiallySet)
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
	else
	{
		IConsoleObject* Obj = IConsoleManager::Get().FindConsoleObject(*Name);
		if (Obj)
		{
			//IConsoleManager::Get().find

			//Object exists so its safe to assume it has any kind of value
			InputType = EConsoleCommandInputType::Value;

			SetBy = GetSetByTCHAR(Obj->GetFlags());

			IConsoleCommand* CCmd = Obj->AsCommand();
			IConsoleVariable* CVar = Obj->AsVariable();
			if (CVar)
			{
				CurrentValue = CVar->GetString();
				ObjType = EConsoleCommandType::CVar;
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
			InputType = EConsoleCommandInputType::None;
		}

		bIsInitiallySet = true;

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

			CurrentValue = CVar->GetString();

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