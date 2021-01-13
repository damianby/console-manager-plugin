// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SConsoleManagerSlateWidget::Construct(const FArguments& InArgs)
{
	CommandsManager = InArgs._CommandsManager;

	GroupsScrollBox = GenerateGroupsScrollBox();

	TSharedRef<SWidget> Content = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("AddGroupButtonText", "Add Group"))
				.ContentPadding(FMargin(4.0f))
				.OnClicked(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked)
	
			]
			+ SVerticalBox::Slot()
				.FillHeight(1)
				[
					GroupsScrollBox.ToSharedRef()
				]
			]
		+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Menu"))
			]

		+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Here commands"))
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString("here command input"))
			]
		];



	ChildSlot
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			Content
		];
}

FReply SConsoleManagerSlateWidget::OnAddGroupButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("CLICKED ADD GROUP"));

	GroupsScrollBox->AddSlot()
		[
			SNew(SButton)
			.Text(FText::FromString("ELO"))
		];

	return FReply::Handled();
}

TSharedRef<SScrollBox> SConsoleManagerSlateWidget::GenerateGroupsScrollBox()
{
	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox);

	const TArray<FString> Commands = CommandsManager.Pin()->GetGroupList();

	for (int i = 0; i < Commands.Num(); i++)
	{
		ScrollBox->AddSlot()
			[
				SNew(SButton)
				.Text(FText::FromString(Commands[i]))
			];
	}


	return ScrollBox;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
