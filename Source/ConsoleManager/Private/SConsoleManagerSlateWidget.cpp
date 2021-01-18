// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SConsoleManagerSlateWidget::Construct(const FArguments& InArgs)
{

	UE_LOG(LogTemp, Warning, TEXT("Construct slate widget"));
	CommandsManager = InArgs._CommandsManager;

	GroupsScrollBox = SNew(SScrollBox);
	GenerateGroupsScrollBox();

	CommandsScrollBox = SNew(SScrollBox);
	GenerateCommandsScrollBox();

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
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString("History"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString("All Commands"))
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1)
			[
				CommandsScrollBox.ToSharedRef()
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

	CommandsManager.Pin()->AddNewGroup("NewGroup");

	GenerateGroupsScrollBox();

	return FReply::Handled();
}

FReply SConsoleManagerSlateWidget::OnSelectGroupClicked(int Id)
{
	UE_LOG(LogTemp, Warning, TEXT("Id: %d"), Id);

	CommandsManager.Pin()->SetActiveGroup(Id);

	GenerateCommandsScrollBox();

	return FReply::Handled();
}

FReply SConsoleManagerSlateWidget::OnSelectCommandClicked(int Id)
{
	const FConsoleCommand& Command = CommandsManager.Pin()->GetConsoleCommand(Id);

	if (!CommandsManager.Pin()->ExecuteCurrentCommand(Id))
	{
		//GenerateCommandsScrollBox();
	}

	return FReply::Handled();
}

void SConsoleManagerSlateWidget::GenerateGroupsScrollBox()
{
	check(GroupsScrollBox.IsValid());

	GroupsScrollBox->ClearChildren();

	const TArray<FString> Commands = CommandsManager.Pin()->GetGroupList();

	for (int i = 0; i < Commands.Num(); i++)
	{
		GroupsScrollBox->AddSlot()
			[
				SNew(SButton)
				.Text(FText::FromString(Commands[i]))
				.OnClicked(this, &SConsoleManagerSlateWidget::OnSelectGroupClicked, i)
			];
	}
}

void SConsoleManagerSlateWidget::GenerateCommandsScrollBox()
{
	check(CommandsScrollBox.IsValid());

	CommandsScrollBox->ClearChildren();

	const TArray<FConsoleCommand>& Commands = CommandsManager.Pin()->GetCurrentCommands();

	for (int i = 0; i < Commands.Num(); i++)
	{
		//////crash!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		const FConsoleCommand& Command = Commands[i];
		UE_LOG(LogTemp, Warning, TEXT("Command: %s %s %s %s"), *Command.Command, *Command.SetBy, *Command.Type, *Command.Value);

		TAttribute<FSlateColor> Value = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([&Command]() {
			if (!Command.IsValid)
			{
				return FSlateColor(FLinearColor(255, 0, 0, 255));
			}
			return FSlateColor(FLinearColor(255, 255, 255, 255));
			}));
		

		TAttribute<FText> CommandValue = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([&Command]() {
			return FText::FromString(Command.Value);
			}));


		CommandsScrollBox->AddSlot()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(Command.Command))
					.ButtonColorAndOpacity(Value)
					.OnClicked(this, &SConsoleManagerSlateWidget::OnSelectCommandClicked, i)
				]
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Command.SetBy))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Command.Type))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(STextBlock)
					.Text(CommandValue)
				]
				
			];
	}

}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
