// Fill out your copyright notice in the Description page of Project Settings.


#include "ConsoleManagerSettings.h"

UConsoleManagerSettings::UConsoleManagerSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
{
	MatchingValuesColor = FLinearColor(0.f, 1.f, 0.f, 1.f);
	NotMatchingValuesColor = FLinearColor(1.f, 0.f, 0.f, 1.f);
}
