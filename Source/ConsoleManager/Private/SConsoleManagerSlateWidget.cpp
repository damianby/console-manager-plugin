// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "ConsoleManagerCommands.h"

#include "Widgets/Layout/SScrollBox.h"

#include "Dialogs/Dialogs.h"
#include "Dialogs/CustomDialog.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SConsoleManagerSlateWidget::Construct(const FArguments& InArgs)
{

	UE_LOG(LogTemp, Warning, TEXT("Construct slate widget"));
	CommandsManager = InArgs._CommandsManager;

	TSharedPtr< SHeaderRow > HeaderRow = SNew(SHeaderRow);
	

	HeaderRow->AddColumn(SHeaderRow::Column("Command")
		[
			SNew(SBorder)
			.Padding(5)
			.Content()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Command"))
			]
		]);

	HeaderRow->AddColumn(SHeaderRow::Column("Value").DefaultLabel(FText::FromString("Value")));

	if (InArgs._DisplayCommandValueType)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("Type").DefaultLabel(FText::FromString("Type")));
	}
	if (InArgs._DisplaySetByValue)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("SetBy").DefaultLabel(FText::FromString("SetBy")));
	}
	HeaderRow->AddColumn(SHeaderRow::Column("Current Value").DefaultLabel(FText::FromString("Current Value")));
	HeaderRow->AddColumn(SHeaderRow::Column("Execute").DefaultLabel(FText::FromString("")));


	CommandsListView = SNew(SListView< TSharedPtr<FConsoleCommand> >)
		.ListItemsSource(&CommandsManager.Pin()->GetCurrentCommandsSharedPtr())
		.ItemHeight(20.0f)
		.OnGenerateRow(this, &SConsoleManagerSlateWidget::OnCommandsRowGenerate)
		.SelectionMode(ESelectionMode::Multi)
		.HeaderRow(HeaderRow);
		

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
			.FillWidth(0.3)
			.Padding(5.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("HistoryButton", "History"))
					.OnClicked(FOnClicked::CreateLambda([=]() {
						CommandsManager.Pin()->SetActiveHistory();
						GenerateCommandsScrollBox();
						CommandsListView->ScrollToBottom();
						UE_LOG(LogTemp, Warning, TEXT("History"));
						return FReply::Handled();
					}))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("ShowAllButton", "Show All"))
					.OnClicked(FOnClicked::CreateLambda([=]() {
						CommandsManager.Pin()->SetActiveAllCommands();
						GenerateCommandsScrollBox();
						CommandsListView->ScrollToBottom();
						UE_LOG(LogTemp, Warning, TEXT("All commands"));
						return FReply::Handled();
						}))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSpacer)
					.Size(FVector2D(0.0f, 15.0f))
					

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
					CommandsListView.ToSharedRef()
					//CommandsScrollBox.ToSharedRef()
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

TSharedRef<ITableRow> SConsoleManagerSlateWidget::OnCommandsRowGenerate(TSharedPtr<FConsoleCommand> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SConsoleCommandListRow, OwnerTable).Item(Item);
}

void SConsoleManagerSlateWidget::GenerateGroupsScrollBox()
{
	check(GroupsScrollBox.IsValid());

	GroupsScrollBox->ClearChildren();

	const TArray<FCommandGroup>& Groups = CommandsManager.Pin()->GetCommandGroups();

	GroupsScrollBox->SetOnMouseButtonUp(FPointerEventHandler::CreateLambda(
		[=](const FGeometry& Geometry, const FPointerEvent& MouseEvent) 
		{
			UE_LOG(LogTemp, Warning, TEXT("IUPPP"));

			return FReply::Handled();
		}));

	

	for (int i = 0; i < Groups.Num(); i++)
	{
		const FCommandGroup& Group = Groups[i];

		TAttribute<FText> GroupName = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([&Group]() {
			return FText::FromString(Group.Name);
			}));


		TSharedRef<SGroupButton> Button =
			SNew(SGroupButton)
			//.Text(GroupName)
			.OnClicked(this, &SConsoleManagerSlateWidget::OnSelectGroupClicked, i)
			.Content()
			[
				SNew(STextBlock)
				.Text(GroupName)
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
			];

			

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
							FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::EditGroup, i),
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
							FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::DuplicateGroup, i),
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
							FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::RemoveGroup, i),
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
	
	CommandsListView->SetListItemsSource(CommandsManager.Pin()->GetCurrentCommandsSharedPtr());
	CommandsListView->RebuildList();


	/*for (int i = 0; i < Commands.Num(); i++)
	{*/
		////////crash!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//const FConsoleCommand& Command = Commands[i];
		//UE_LOG(LogTemp, Warning, TEXT("Command: %s %s %s %s"), *Command.Command, *Command.SetBy, *Command.Type, *Command.Value);

		//TAttribute<FSlateColor> Value = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([&Command]() {
		//	if (!Command.IsValid)
		//	{
		//		return FSlateColor(FLinearColor(255, 0, 0, 255));
		//	}
		//	return FSlateColor(FLinearColor(255, 255, 255, 255));
		//	}));
		//

		//TAttribute<FText> CommandValue = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([&Command]() {
		//	return FText::FromString(Command.Value);
		//	}));

	//}

}

void SConsoleManagerSlateWidget::RemoveGroup(int Id)
{
	FText Title = FText::FromString("Remove Group");

	EAppReturnType::Type Output = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("RemoveGroupDialog", "Do you want to remove group?"), &Title);

	switch (Output)
	{
	case EAppReturnType::Type::Yes:
		CommandsManager.Pin()->RemoveGroup(Id);
		GenerateGroupsScrollBox();
		GenerateCommandsScrollBox();
		break;

	case EAppReturnType::Type::No:
	default:
		break;
	}
}

void SConsoleManagerSlateWidget::DuplicateGroup(int Id)
{
	CommandsManager.Pin()->DuplicateGroup(Id);
	GenerateGroupsScrollBox();
}

void SConsoleManagerSlateWidget::EditGroup(int Id)
{
	const TArray<FString> Groups = CommandsManager.Pin()->GetGroupList();


	TSharedRef<SEditableTextBox> Widget =
		SNew(SEditableTextBox)
		.HintText(FText::FromString("New Name"))
		.Text(FText::FromString(Groups[Id]));

	TSharedRef<SWidget> ContentWidget =
		SNew(SBox)
		.Padding(FMargin(0.f, 15.f, 0.f, 0.f))
		.WidthOverride(250)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 10.f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("Choose new name for group %s"), *Groups[Id])))
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				Widget
			]
		];


	TSharedRef<SCustomDialog> EditDialog = SNew(SCustomDialog)
		.Title(FText(LOCTEXT("EditGroupDialog_Title", "Edit Group")))
		.DialogContent(ContentWidget)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});

	

	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = EditDialog->ShowModal();

	switch (ButtonPressed)
	{
	case 0:
		CommandsManager.Pin()->RenameGroup(Id, Widget->GetText().ToString());
		break;
	}

	//SGenericDialogWidget::OpenDialog(LOCTEXT("EditGroupDialog_Title", "Edit Group"), Widget, SGenericDialogWidget::FArguments(), true);
}


TSharedRef<SWidget> SConsoleCommandListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	
	if (ColumnName.IsEqual(FName(TEXT("Command"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->Command));
	}
	if (ColumnName.IsEqual(FName(TEXT("Value"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->Value));
	}

	if (ColumnName.IsEqual(FName(TEXT("Type"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->Type));
	}

	if (ColumnName.IsEqual(FName(TEXT("SetBy"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->SetBy));
	}

	if (ColumnName.IsEqual(FName(TEXT("Current Value"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->CurrentValue));
	}

	if (ColumnName.IsEqual(FName(TEXT("Execute"))))
	{
		return SNew(SButton).Text(FText::FromString("Execute"))
			.OnClicked(FOnClicked::CreateLambda([=]() {
			UE_LOG(LogTemp, Warning, TEXT("Value of click: %s"), *Item->CurrentValue);

			//auto Widget = OwnerTablePtr.Pin()->WidgetFromItem(Item);

			//if (Widget)
			//{
			//	Widget->ResetRow();
			//}


			return FReply::Unhandled();
				}));
	}

	return SNullWidget::NullWidget;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
