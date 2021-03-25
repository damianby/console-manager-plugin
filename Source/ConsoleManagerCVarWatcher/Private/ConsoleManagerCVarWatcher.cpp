// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "ConsoleManagerCVarWatcher.h"

#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerCVarWatcherModule"

void FConsoleManagerCVarWatcherModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("CVar watcher started!"));
	
	
	const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir());
	
	UE_LOG(LogTemp, Warning, TEXT("Project Path: %s"), *ProjectPath);

	//DECLARE_MULTICAST_DELEGATE_FourParams(FOnApplyCVarFromIni, const TCHAR* /*SectionName*/, const TCHAR* /*IniFilename*/, uint32 /*SetBy*/, bool /*bAllowCheating*/);
	

	//DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConfigSectionRead, const TCHAR* /*IniFilename*/, const TCHAR* /*SectionName*/);
	//static FOnConfigSectionRead OnConfigSectionRead;

	//FCoreDelegates::OnConfigSectionRead.AddLambda(
	//	[=](const TCHAR* IniFileName, const TCHAR* SectionName)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("READ: : : Section name: %s | File name: %s"), SectionName, IniFileName);

	//	}
	//);

	FCoreDelegates::OnApplyCVarFromIni.AddLambda(
		[=](const TCHAR* SectionName, const TCHAR* IniFileName, uint32 SetBy, bool AllowCheating)
		{

			bool bIsConfigFolder = FPaths::IsUnderDirectory(IniFileName, ProjectPath);

			UE_LOG(LogTemp, Warning, TEXT("Is under dir:  %d | Section name: %s | File name: %s"), bIsConfigFolder, SectionName, IniFileName);

			//UE_LOG(LogConfig, Log, TEXT("Applying CVar settings from Section [%s] File [%s]"), InSectionName, InIniFilename);

			if (FConfigSection* Section = GConfig->GetSectionPrivate(SectionName, false, true, IniFileName))
			{
				for (FConfigSectionMap::TConstIterator It(*Section); It; ++It)
				{
					const FString& KeyString = It.Key().GetPlainNameString();
					const FString& ValueString = It.Value().GetValue();

					IniVariables.Add(TPair<FString, FString>(KeyString, ValueString));

					UE_LOG(LogTemp, Warning, TEXT("Command: %s %s"), *KeyString, *ValueString);

					Commands.Add(KeyString + " " + ValueString);
					//OnSetCVarFromIniEntry(InIniFilename, *KeyString, *ValueString, SetBy, bAllowCheating);
				}
			}
			UE_LOG(LogTemp, Warning, TEXT(""));

			const FString Path = FPaths::GeneratedConfigDir() + TEXT("testcvars.txt");
			FFileHelper::SaveStringArrayToFile(Commands, *Path);


		});
}

void FConsoleManagerCVarWatcherModule::ShutdownModule()
{

}

IMPLEMENT_MODULE(FConsoleManagerCVarWatcherModule, ConsoleManagerCVarWatcher)