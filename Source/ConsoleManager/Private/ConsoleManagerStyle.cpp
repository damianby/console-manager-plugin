// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#include "ConsoleManagerStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FConsoleManagerStyle::StyleInstance = NULL;

/**
 * Helper class for styles
*/
class FSlateConsoleManagerStyle : public FSlateStyleSet
{
public:
	FSlateConsoleManagerStyle(const FName& InStyleSetName)
		: FSlateStyleSet(InStyleSetName)

		, DefaultMatchingValues_Ref(MakeShareable(new FLinearColor(0.f, 1.f, 0.f, 1.f)))
		, DefaultNotMatchingValues_Ref(MakeShareable(new FLinearColor(1.f, 0.f, 0.f, 1.f)))
	{

	}


	static void SetColor(const TSharedRef<FLinearColor>& Source, const FLinearColor& Value)
	{
		Source->R = Value.R;
		Source->G = Value.G;
		Source->B = Value.B;
		Source->A = Value.A;
	}


	const TSharedRef<FLinearColor> DefaultMatchingValues_Ref;
	const TSharedRef<FLinearColor> DefaultNotMatchingValues_Ref;

};


void FConsoleManagerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FConsoleManagerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FConsoleManagerStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ConsoleManagerStyle"));
	return StyleSetName;
}

void FConsoleManagerStyle::SetNotMachingValuesColor(const FLinearColor& NewColor)
{
	TSharedPtr<FSlateConsoleManagerStyle> Style = StaticCastSharedPtr<FSlateConsoleManagerStyle>(StyleInstance);
	check(Style.IsValid());

	FSlateConsoleManagerStyle::SetColor(Style->DefaultNotMatchingValues_Ref, NewColor);
}

void FConsoleManagerStyle::SetMatchingValuesColor(const FLinearColor& NewColor)
{
	TSharedPtr<FSlateConsoleManagerStyle> Style = StaticCastSharedPtr<FSlateConsoleManagerStyle>(StyleInstance);
	check(Style.IsValid());

	FSlateConsoleManagerStyle::SetColor(Style->DefaultMatchingValues_Ref, NewColor);
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FConsoleManagerStyle::Create()
{
	TSharedRef< FSlateConsoleManagerStyle > Style = MakeShareable(new FSlateConsoleManagerStyle("ConsoleManagerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources"));

	const FSlateColor DefaultMatchingValues(Style->DefaultMatchingValues_Ref);
	const FSlateColor DefaultNotMatchingValues(Style->DefaultNotMatchingValues_Ref);

	Style->Set("MatchingValues", DefaultMatchingValues);
	Style->Set("NotMatchingValues", DefaultNotMatchingValues);

	//Style->Set("ConsoleManager.EmptyButton", new FButtonStyle());

	Style->Set("ConsoleManager.OpenTab", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ConsoleManager.OpenTab.Small", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon20x20));
	Style->Set("ConsoleManager.ExecAction", new IMAGE_BRUSH(TEXT("go"), Icon16x16));
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FConsoleManagerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FConsoleManagerStyle::Get()
{
	return *StyleInstance;
}
