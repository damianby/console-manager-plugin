// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "ConsoleManagerSettings.h"

UConsoleManagerSettings::UConsoleManagerSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
{
	MatchingValuesColor = FLinearColor(0.f, 1.f, 0.f, 1.f);
	NotMatchingValuesColor = FLinearColor(1.f, 0.f, 0.f, 1.f);
}
