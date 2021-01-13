// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHelper.h"

#include "Misc/FileHelper.h"


void FileHelper::LoadConsoleHistory(FCommandGroup& OutGroup)
{
	FConfigFile Ini;

	const FString ConfigPath = FPaths::GeneratedConfigDir() + TEXT("ConsoleHistory.ini");
	
	Ini.Read(ConfigPath);
	const FString SectionName = TEXT("ConsoleHistory");
	const FName KeyName = TEXT("History");

	OutGroup.Name = SectionName;
	OutGroup.Commands.Empty();

	for (const auto& ConfigPair : Ini)
	{
		FString HistoryKey;
		if (ConfigPair.Key == SectionName)
		{
			// uses empty HistoryKey
			//UE_LOG(LogTemp, Warning, TEXT("History empty"));

		}
		else if (ConfigPair.Key.StartsWith(SectionName))
		{
			HistoryKey = ConfigPair.Key.Mid(SectionName.Len());
			//UE_LOG(LogTemp, Warning, TEXT("History key mid: %s"), *HistoryKey);

		}
		else
		{
			continue;
		}

		//TArray<FString>& HistoryEntries = HistoryEntriesMap.FindOrAdd(HistoryKey);
		for (const auto& ConfigSectionPair : ConfigPair.Value)
		{
			if (ConfigSectionPair.Key == KeyName)
			{
				//HistoryEntries.Add(ConfigSectionPair.Value.GetValue());
				//UE_LOG(LogTemp, Warning, TEXT("History starts: %s"), *ConfigSectionPair.Value.GetValue());
				OutGroup.Commands.Add(ConfigSectionPair.Value.GetValue());
			}
		}
	}
}


void FileHelper::ProcessCommandsFile(const FString& Contents, TArray<FCommandGroup>& OutGroups)
{
	OutGroups.Empty();

	const TCHAR* Ptr = Contents.Len() > 0 ? *Contents : nullptr;
	FCommandGroup* CurrentSection = nullptr;
	bool Done = false;
	while (!Done && Ptr != nullptr)
	{
		// Advance past new line characters
		while (*Ptr == '\r' || *Ptr == '\n')
		{
			Ptr++;
		}
		// read the next line
		FString TheLine;
		int32 LinesConsumed = 0;
		FParse::LineExtended(&Ptr, TheLine, LinesConsumed, false);
		if (Ptr == nullptr || *Ptr == 0)
		{
			Done = true;
		}
		TCHAR* Start = const_cast<TCHAR*>(*TheLine);

		// Strip trailing spaces from the current line
		while (*Start && FChar::IsWhitespace(Start[FCString::Strlen(Start) - 1]))
		{
			Start[FCString::Strlen(Start) - 1] = 0;
		}

		// If the first character in the line is [ and last char is ], this line indicates a section name
		if (*Start == '[' && Start[FCString::Strlen(Start) - 1] == ']')
		{
			// Remove the brackets
			Start++;
			Start[FCString::Strlen(Start) - 1] = 0;

			// If we don't have an existing section by this name, add one

			CurrentSection = &OutGroups.AddDefaulted_GetRef();
			CurrentSection->Name = Start;
		}

		// Otherwise, if we're currently inside a section, and we haven't reached the end of the stream
		else if (CurrentSection && *Start)
		{
			//TCHAR* Value = 0;
			TCHAR* Value = Start;


			//// ignore [comment] lines that start with ;
			//if (*Start != (TCHAR)';')
			//{
			//	Value = FCString::Strstr(Start, TEXT("="));
			//}

			// Ignore any lines that don't contain a key-value pair
			if (Value || true)
			{
				//// Terminate the propertyname, advancing past the =
				//*Value++ = 0;

				//// strip leading whitespace from the property name
				//while (*Start && FChar::IsWhitespace(*Start))
				//	Start++;

				//// Strip trailing spaces from the property name.
				//while (*Start && FChar::IsWhitespace(Start[FCString::Strlen(Start) - 1]))
				//	Start[FCString::Strlen(Start) - 1] = 0;

				// Strip leading whitespace from the property value
				while (*Value && FChar::IsWhitespace(*Value))
					Value++;

				// strip trailing whitespace from the property value
				while (*Value && FChar::IsWhitespace(Value[FCString::Strlen(Value) - 1]))
					Value[FCString::Strlen(Value) - 1] = 0;

				// If this line is delimited by quotes
				if (*Value == '\"')
				{
					FString ProcessedValue;
					FParse::QuotedString(Value, ProcessedValue);

					// Add this pair to the current FConfigSection
					CurrentSection->Commands.Add(ProcessedValue);
				}
				else
				{
					// Add this pair to the current FConfigSection
					CurrentSection->Commands.Add(Value);
				}
			}
		}
	}
}

bool FileHelper::ReadCommandFile(const FString& Path, TArray<FCommandGroup>& OutGroups)
{
	if (!FPaths::FileExists(Path))
	{
		return false;
	}


	FString Contents;
	if (!FFileHelper::LoadFileToString(Contents, *Path))
	{
		return false;
	}

	ProcessCommandsFile(Contents, OutGroups);

	return true;
}

bool FileHelper::SaveCommandFile(const FString& Path, const TArray<FCommandGroup>& Groups, bool bOverwrite)
{
	if (FPaths::FileExists(Path) && !bOverwrite)
	{
		return false;
	}

	FString Final;
	
	for (const FCommandGroup& Group : Groups)
	{
		FString Text;

		Text.Appendf(TEXT("[%s]") LINE_TERMINATOR, *Group.Name);

		for (const FString& Command : Group.Commands)
		{
			Text.Appendf(TEXT("%s") LINE_TERMINATOR, *Command);
		}
		//For nice formatting add terminator at end of section
		Text.Append(LINE_TERMINATOR);

		Final += Text;
	}

	return FFileHelper::SaveStringToFile(Final, *Path);
}

void FileHelper::PrintGroups_Debug(const TArray<FCommandGroup>& Groups)
{
	for (const FCommandGroup& Group : Groups)
	{
		UE_LOG(LogTemp, Warning, TEXT("Group Name: %s"), *Group.Name);

		int i = 1;
		for (const FString& Command : Group.Commands)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d. %s"), i, *Command);
			i++;
		}
	}
}
