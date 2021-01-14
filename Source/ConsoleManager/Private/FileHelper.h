// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommandStructs.h"
/**
 * 
 */
class FileHelper
{
public:

	/**
	 * @brief	Loads console history from default project path Saved\Config\ConsoleHistory.ini
	 * @param OutGroup	CommandGroup containing console history
	*/
	static void LoadConsoleHistory(FCommandGroup& OutGroup);

	/**
	 * @brief	Command file is almost the same as config file expect it uses lines as values instead of key=value
	 * 
	 * @see		ConfigCacheIni.cpp at void FConfigFile::ProcessInputFileContents(const FString& Contents)
	 * 
	 * @param Contents		Input string of loaded commands file
	 * @param OutGroups		Found group sections with commands
	*/
	static void ProcessCommandsFile(const FString& Contents, TArray<FCommandGroup>& OutGroups);

	/**
	 * @brief Loads path at given path and processes its input, if file is invalid OutGroups will not be modified
	 * 
	 * @param Path				Input file path
	 * @param OutGroups			Output command groups from input file
	*/
	static bool ReadCommandFile(const FString& Path, TArray<FCommandGroup>& OutGroups);

	/**
	 * @brief				Save groups to file
	 * @param Path			Out file path
	 * @param Groups		Input groups
	 * @param bOverwrite	If true overwrite if file exists
	 * @return 
	*/
	static bool SaveCommandFile(const FString& Path, const TArray<FCommandGroup>& Groups, bool bOverwrite = true);




	/**
	 * @brief Debug print to log
	 * @param Groups 
	*/
	static void PrintGroups_Debug(const TArray<FCommandGroup>& Groups);
};
