// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "ConsoleManagerCommands.h"

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



	//FConsoleManagerCommands::Get().GroupContextMenu = MakeShareable(new FUICommandList);

	FConsoleManagerCommands::Get().GroupContextMenu->MapAction(
		FConsoleManagerCommands::Get().NewGroupCommand,
		FExecuteAction::CreateLambda([=]() {
			UE_LOG(LogTemp, Warning, TEXT("LAmbda"))
			}),
		FCanExecuteAction());

	
	TSharedRef<SWidget> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[	
				SNew(SSearchBox)
				.HintText(FText::FromString("Search/Filter"))
			]
		]

		+ SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("HistoryButton", "History"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("ShowAllButton", "Show All"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
					.Size(FVector2D(0.0f, 15.0f))
					

				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("AddGroupButtonText", "Add Group"))
					.ContentPadding(FMargin(4.0f))
					//.OnClicked(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked)

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
			]
		];
		



	ChildSlot
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			Content
		];
}

//FReply SConsoleManagerSlateWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
//{
//	return FReply::Unhandled();
//
//	FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());
//	
//	{
//		MenuBuilder.BeginSection("Group", LOCTEXT("GroupContextMenu_Header_Group", "Group"));
//		{
//			FUIAction Action_NewGroup(
//
//				FExecuteAction::CreateSP(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked),
//				FCanExecuteAction()
//			);
//
//			MenuBuilder.AddMenuEntry
//			(
//				LOCTEXT("GroupContextMenu_NewGroup", "Add group"),
//				LOCTEXT("GroupContextMenu_NewGroup_Desc", "Creates new group"),
//				FSlateIcon(),
//				Action_NewGroup,
//				NAME_None,
//				EUserInterfaceActionType::Button
//			);
//		}
//		MenuBuilder.EndSection();
//	}
//
//	
//
//	UE_LOG(LogTemp, Warning, TEXT("Mouse btn up"));
//
//	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
//	{
//		TSharedRef<SWidget> MenuContents = MenuBuilder.MakeWidget();
//		FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
//		FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
//
//		return FReply::Handled();
//	}
//	else
//	{
//		return FReply::Unhandled();
//	}
//
//
//
//}

void SConsoleManagerSlateWidget::OnAddGroupButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("CLICKED ADD GROUP"));

	CommandsManager.Pin()->AddNewGroup("NewGroup");

	GenerateGroupsScrollBox();

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

	GroupsScrollBox->SetOnMouseButtonUp(FPointerEventHandler::CreateLambda(
		[=](const FGeometry& Geometry, const FPointerEvent& MouseEvent) 
		{
			UE_LOG(LogTemp, Warning, TEXT("IUPPP"));

			return FReply::Handled();
		}));

	

	for (int i = 0; i < Commands.Num(); i++)
	{
		TSharedRef<SGroupButton> Button =
			SNew(SGroupButton)
			.Text(FText::FromString(Commands[i]))
			.OnClicked(this, &SConsoleManagerSlateWidget::OnSelectGroupClicked, i);


		//FPointerEventHandler MouseButtonDownHandler;
		//MouseButtonDownHandler.BindLambda

		Button->RightClickDelegate.BindLambda(
			[=](const FGeometry& Geometry, const FPointerEvent& MouseEvent) 
			{

				UE_LOG(LogTemp, Warning, TEXT("Group clicked!"));
				FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());

				{
					MenuBuilder.BeginSection("Group", LOCTEXT("GroupContextMenu_Header_Group", "Group"));
					{
						FUIAction Action_EditGroup(

							FExecuteAction::CreateSP(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked),
							FCanExecuteAction()
						);

						MenuBuilder.AddMenuEntry
						(
							LOCTEXT("GroupContextMenu_EditGroup", "Edit"),
							LOCTEXT("GroupContextMenu_EditGroup_Desc", "Edit group"),
							FSlateIcon(),
							Action_EditGroup,
							NAME_None,
							EUserInterfaceActionType::Button
						);


						FUIAction Action_DuplicateGroup(

							FExecuteAction::CreateSP(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked),
							FCanExecuteAction()
						);

						MenuBuilder.AddMenuEntry
						(
							LOCTEXT("GroupContextMenu_DuplicateGroup", "Duplicate"),
							LOCTEXT("GroupContextMenu_DuplicateGroup_Desc", "Duplicate group"),
							FSlateIcon(),
							Action_DuplicateGroup,
							NAME_None,
							EUserInterfaceActionType::Button
						);
						FUIAction Action_RemoveGroup(

							FExecuteAction::CreateSP(this, &SConsoleManagerSlateWidget::OnAddGroupButtonClicked),
							FCanExecuteAction()
						);

						MenuBuilder.AddMenuEntry
						(
							LOCTEXT("GroupContextMenu_RemoveGroup", "Remove"),
							LOCTEXT("GroupContextMenu_RemoveGroup_Desc", "Remove group"),
							FSlateIcon(),
							Action_RemoveGroup,
							NAME_None,
							EUserInterfaceActionType::Button
						);

					}
					MenuBuilder.EndSection();
				}


				if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					TSharedRef<SWidget> MenuContents = MenuBuilder.MakeWidget();
					FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
					FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

					return FReply::Handled();
				}
				else
				{
					return FReply::Unhandled();
				}
			});

		GroupsScrollBox->AddSlot()
			[
				Button
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
