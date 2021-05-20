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
		, DefaultButtonHover_Ref(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
		, DefaultButtonPress_Ref(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
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

	const TSharedRef<FLinearColor> DefaultButtonHover_Ref;
	const TSharedRef<FLinearColor> DefaultButtonPress_Ref;

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

void FConsoleManagerStyle::SetCommandsFontSize(int32 NewSize)
{
	check(StyleInstance);

	StyleInstance->Set("Fonts.Commands", FCoreStyle::GetDefaultFontStyle("Regular", NewSize));
	StyleInstance->Set("Fonts.FontSize", static_cast<float>(NewSize));
	
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )


#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon27x27(27.0f, 27.0f);

TSharedRef< FSlateStyleSet > FConsoleManagerStyle::Create()
{
	TSharedRef< FSlateConsoleManagerStyle > Style = MakeShareable(new FSlateConsoleManagerStyle("ConsoleManagerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ConsoleManager")->GetBaseDir() / TEXT("Resources"));

	const FSlateColor DefaultMatchingValues(Style->DefaultMatchingValues_Ref);
	const FSlateColor DefaultNotMatchingValues(Style->DefaultNotMatchingValues_Ref);

	const FSlateColor DefaultButtonHover(Style->DefaultButtonHover_Ref);
	const FSlateColor DefaultButtonPress(Style->DefaultButtonPress_Ref);

	Style->Set("MatchingValues", DefaultMatchingValues);
	Style->Set("NotMatchingValues", DefaultNotMatchingValues);

	//Style->Set("ConsoleManager.EmptyButton", new FButtonStyle());

	Style->Set("ConsoleManager.OpenTab", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ConsoleManager.OpenTab.Small", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon20x20));

	FSlateImageBrush* ExecActionBrush = new IMAGE_BRUSH(TEXT("Execute_16"), Icon16x16);
	ExecActionBrush->DrawAs = ESlateBrushDrawType::Box;
	ExecActionBrush->Margin = FMargin(0);

	Style->Set("ConsoleManager.ExecAction", ExecActionBrush);


	FSlateImageBrush* NoteBrush = new IMAGE_BRUSH(TEXT("Note_16"), Icon16x16);
	NoteBrush->DrawAs = ESlateBrushDrawType::Box;
	NoteBrush->Margin = FMargin(0);

	Style->Set("Icons.Note", NoteBrush);


	Style->Set("Icons.Settings", new IMAGE_BRUSH(TEXT("Settings_40"), Icon27x27));
	Style->Set("Icons.Save", new IMAGE_BRUSH(TEXT("SaveAll_40"), Icon27x27));
	Style->Set("Icons.Revert", new IMAGE_BRUSH(TEXT("Revert_40"), Icon27x27));
	Style->Set("Icons.Snapshot", new IMAGE_BRUSH(TEXT("Snapshot_40"), Icon27x27));
	Style->Set("Icons.Remove", new IMAGE_BRUSH(TEXT("Remove_40"), Icon27x27));
	Style->Set("Icons.Add", new IMAGE_BRUSH(TEXT("Add_40"), Icon27x27));
	Style->Set("Icons.Duplicate", new IMAGE_BRUSH(TEXT("Duplicate_40"), Icon27x27));
	Style->Set("Icons.Warning", new IMAGE_BRUSH(TEXT("Warning_16"), Icon16x16));
	Style->Set("Icons.Unknown", new IMAGE_BRUSH(TEXT("Unknown_16"), Icon16x16));
	Style->Set("Icons.Error", new IMAGE_BRUSH(TEXT("Error_16"), Icon16x16));


	Style->Set("MenuButtonHoverTint", FSlateColor(FLinearColor(0, 0, 0, 0.7f)));
	Style->Set("MenuButtonPressTint", FSlateColor(FLinearColor(0, 0, 0, 0.5f)));

	// Invisible buttons, borders, etc.
	const FButtonStyle MenuButton = FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateColorBrush(DefaultButtonHover.GetSpecifiedColor()))
		//.SetHovered(FSlateColorBrush(DefaultButtonHover.GetSpecifiedColor()))
		.SetPressed(FSlateColorBrush(DefaultButtonPress.GetSpecifiedColor()))
		//.SetPressed(FSlateColorBrush(DefaultButtonPress.GetSpecifiedColor()))
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f));

	
	Style->Set("Image.GlobalPresetButton", new IMAGE_BRUSH(TEXT("ButtonSolo_Idle"), FVector2D(150, 34)));

	// Convenient transparent/invisible elements
	{
		Style->Set("NoBrush", new FSlateNoResource());
		Style->Set("NoBorder", new FSlateNoResource());
		Style->Set("NoBorder.Normal", new FSlateNoResource());
		Style->Set("NoBorder.Hovered", new FSlateNoResource());
		Style->Set("NoBorder.Pressed", new FSlateNoResource());
		Style->Set("MenuButton", MenuButton);
	}

	// Invisible buttons, borders, etc.
	const FButtonStyle NoBorder = FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
		.SetPressedPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f));

	Style->Set("EmptyButton", NoBorder);
	
	
	Style->Set("Fonts.Commands", FCoreStyle::GetDefaultFontStyle("Regular", 9));
	Style->Set("Fonts.FontSize", 9.0f);


	// Normal Text
	const FTextBlockStyle GlobalPresetButtonFont = FTextBlockStyle()
		.SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 25, "Regular"))
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetShadowOffset(FVector2D::ZeroVector)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f));

	Style->Set("GlobalPresetButtonFont", GlobalPresetButtonFont);

	//const FButtonStyle GlobalPresetButton = FButtonStyle()
	//	.SetNormal(IMAGE_BRUSH("ButtonSolo_Idle", FVector2D(150, 34)))
	//	.SetHovered(FSlateColorBrush(DefaultButtonHover.GetSpecifiedColor()))
	//	//.SetHovered(FSlateColorBrush(DefaultButtonHover.GetSpecifiedColor()))
	//	.SetPressed(FSlateColorBrush(DefaultButtonPress.GetSpecifiedColor()))
	//	//.SetPressed(FSlateColorBrush(DefaultButtonPress.GetSpecifiedColor()))
	//	.SetNormalPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f))
	//	.SetPressedPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f));
	
	/* Default Style for a toggleable button */
	const FCheckBoxStyle GlobalPresetToggleButton = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		//.SetBorderBackgroundColor(FSlateColor(FLinearColor(1.0f,0,0,1.0f)))
		.SetUncheckedImage(IMAGE_BRUSH(TEXT("ButtonSolo_Idle"), FVector2D(150, 34)))
		.SetUncheckedHoveredImage(IMAGE_BRUSH(TEXT("ButtonSolo_Idle"), FVector2D(150, 34), FLinearColor(1.f, 1.f, 1.f, 0.7f)))
		.SetUncheckedPressedImage(IMAGE_BRUSH(TEXT("ButtonSolo_Idle"), FVector2D(150, 34), FLinearColor(1.f, 1.f, 1.f, 0.5f)))
		.SetCheckedImage(IMAGE_BRUSH(TEXT("ButtonSolo_Clicked"), FVector2D(150, 34)))
		.SetCheckedHoveredImage(IMAGE_BRUSH(TEXT("ButtonSolo_Clicked"), FVector2D(150, 34), FLinearColor(1.f, 1.f, 1.f, 0.7f)))
		.SetCheckedPressedImage(IMAGE_BRUSH(TEXT("ButtonSolo_Clicked"), FVector2D(150, 34), FLinearColor(1.f, 1.f, 1.f, 0.5f)));

	Style->Set("GlobalPresetToggleButton", GlobalPresetToggleButton);


	const FCheckBoxStyle SinglePresetToggleButton = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.f)))
		.SetUncheckedHoveredImage(FSlateColorBrush(FLinearColor(0.728f / 4.f, 0.364f / 4.f, 0.003f / 4.f)))
		.SetUncheckedPressedImage(FSlateColorBrush(FLinearColor(0.728f / 2.f, 0.364f / 2.f, 0.003f / 2.f)))
		.SetCheckedImage(FSlateColorBrush(FLinearColor(0.728f, 0.364f, 0.003f)))
		.SetCheckedHoveredImage(FSlateColorBrush(FLinearColor(0.728f, 0.364f, 0.003f)))
		.SetCheckedPressedImage(FSlateColorBrush(FLinearColor(0.728f, 0.364f, 0.003f)));

	Style->Set("SinglePresetToggleButton", SinglePresetToggleButton);

	const FTableRowStyle DefaultTableRowStyle = FTableRowStyle(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
		.SetEvenRowBackgroundBrush(FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.1f)))
		.SetOddRowBackgroundBrush(FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.2f)));

	Style->Set("TableView.Row", DefaultTableRowStyle);

	//In order to bind the thumbnail to our class we need to type ClassThumbnail.X where X is the name of the C++ class of the asset
	Style->Set("ClassThumbnail.CommandsContainer", new IMAGE_BRUSH(TEXT("AssetIcon_128"), FVector2D(128.f, 128.f)));

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
