// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"


class FConsoleManagerCVarWatcherModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


	/**
	 * @brief Provide singleton like access to this module
	 * @return Module
	*/
	static inline FConsoleManagerCVarWatcherModule& GetModule()
	{
		static const FName ModuleName = "ConsoleManagerCVarWatcher";
		return FModuleManager::LoadModuleChecked< FConsoleManagerCVarWatcherModule >(ModuleName);
	}


private:
	TArray<TPair<FString, FString> > IniVariables;

	TArray<FString> Commands;
};


