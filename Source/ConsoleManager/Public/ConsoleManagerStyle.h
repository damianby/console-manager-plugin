// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FConsoleManagerStyle
{
public:

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

	static void SetNotMachingValuesColor(const FLinearColor& NewColor);
    static void SetMatchingValuesColor(const FLinearColor& NewColor);
	static void SetCommandsFontSize(int32 NewSize);

private:

	static TSharedRef< class FSlateStyleSet > Create();

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};