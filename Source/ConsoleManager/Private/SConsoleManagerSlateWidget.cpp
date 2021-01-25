// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "ConsoleManagerCommands.h"

#include "Widgets/Layout/SScrollBox.h"

#include "Widgets/Layout/SGridPanel.h"
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

	

	HeaderRow->AddColumn(SHeaderRow::Column("Value")
		.DefaultLabel(FText::FromString("Execute Value"))
	);

	if (InArgs._DisplayCommandValueType)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("Type")
			.DefaultLabel(FText::FromString("Type"))
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center)
			.FixedWidth(80)
		);

	}
	if (InArgs._DisplaySetByValue)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("SetBy")
			.DefaultLabel(FText::FromString("SetBy"))
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center)
			.FixedWidth(130)
		);

	}
	HeaderRow->AddColumn(SHeaderRow::Column("Current Value")
		.DefaultLabel(FText::FromString("Current Value"))
		.HAlignCell(EHorizontalAlignment::HAlign_Center)
		.HAlignHeader(EHorizontalAlignment::HAlign_Center)
		.VAlignCell(EVerticalAlignment::VAlign_Center)
	);
		

	HeaderRow->AddColumn(SHeaderRow::Column("Execute")
		.DefaultLabel(FText::FromString(""))
		.FixedWidth(50)
	);


	CommandsListView = SNew(SListView< TSharedPtr<FConsoleCommand> >)
		.ListItemsSource(&CommandsManager.Pin()->GetCurrentCommandsSharedPtr())
		.ItemHeight(20.0f)
		.OnGenerateRow(this, &SConsoleManagerSlateWidget::OnCommandsRowGenerate)
		.SelectionMode(ESelectionMode::Multi)
		.HeaderRow(HeaderRow)
		.OnContextMenuOpening_Raw(this, &SConsoleManagerSlateWidget::GetListViewContextMenu);
		

	GroupsScrollBox = SNew(SScrollBox);

	CommandsScrollBox = SNew(SScrollBox);

	//FConsoleManagerCommands::Get().GroupContextMenu = MakeShareable(new FUICommandList);


	FPointerEventHandler GroupsScrollBoxRightClick;

	GroupsScrollBoxRightClick.BindLambda(
		[=](const FGeometry& Geometry, const FPointerEvent& MouseEvent)
		{
			FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());

			{
				MenuBuilder.BeginSection("Group", LOCTEXT("GroupContextMenu_Header_Group", "Group"));
				{

					MenuBuilder.AddMenuEntry
					(
						LOCTEXT("GroupContextMenu_NewGroup", "Add New"),
						LOCTEXT("GroupContextMenu_NewGroup_Desc", "Add new group"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda(
							[=]() {
								HandleNewGroup();
								

							}),
							FCanExecuteAction()),
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
		
			return FReply::Unhandled();
			
		});



	FConsoleManagerCommands::Get().GroupContextMenu->MapAction(
		FConsoleManagerCommands::Get().NewGroupCommand,
		FExecuteAction::CreateLambda([=]() {
			UE_LOG(LogTemp, Warning, TEXT("LAmbda"))
			}),
		FCanExecuteAction());

	//Validate all console commands to check if any existing in AllCommands //git
	TSharedRef<SWidget> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[	
				SAssignNew(SearchBox, SSearchBox)
				.HintText(FText::FromString("Search/Filter"))
				.OnTextChanged_Lambda(
						[=](const FText& NewText) {
							const TArray<TSharedPtr<FConsoleCommand>>&  Commands = CommandsManager.Pin()->GetCurrentCommandsSharedPtr();
							
							if (NewText.IsEmpty())
							{
								CommandsListView->SetListItemsSource(Commands);
								CommandsListView->RebuildList();
								return;
							}

							FilteredListView.Empty();
							for (const auto& Command : Commands)
							{
								FString Part = NewText.ToString();
								if (Command->Name.Contains(Part))
								{
									FilteredListView.Add(Command);
								}
								
							}
							CommandsListView->SetListItemsSource(FilteredListView);
							CommandsListView->RebuildList();
						}
					)
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
					.OnClicked(FOnClicked::CreateLambda([=]() {
						bIsAllCommands = false;
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
						bIsAllCommands = true;
						CommandsManager.Pin()->SetActiveAllCommands();
						GenerateCommandsScrollBox();

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
					SNew(SBorder)
					.BorderBackgroundColor(FSlateColor(FLinearColor(255, 0, 0)))
					.Visibility(EVisibility::SelfHitTestInvisible)
					.OnMouseButtonUp(GroupsScrollBoxRightClick)
					[
						GroupsScrollBox.ToSharedRef()
					]
					
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


	GenerateGroupsScrollBox();
	GenerateCommandsScrollBox();

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

	bIsAllCommands = false;

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
	Item->Refresh();

	

	return SNew(SConsoleCommandListRow, OwnerTable).Item(Item).bIsValid(Item->IsValid);
}

void SConsoleManagerSlateWidget::GenerateGroupsScrollBox()
{
	check(GroupsScrollBox.IsValid());

	GroupsScrollBox->ClearChildren();

	const TArray<FCommandGroup>& Groups = CommandsManager.Pin()->GetCommandGroups();

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

						MenuBuilder.AddMenuEntry
						(
							LOCTEXT("GroupContextMenu_NewGroup", "Add New"),
							LOCTEXT("GroupContextMenu_NewGroup_Desc", "Add new group"),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateLambda(
								[=]() {
									HandleNewGroup();
								}),
								FCanExecuteAction()),
							NAME_None,
							EUserInterfaceActionType::Button
						);

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
	if (bIsAllCommands)
	{
		CommandsListView->GetHeaderRow()->RemoveColumn("Value");
	}
	else
	{

		if(!CommandsListView->GetHeaderRow()->IsColumnGenerated("Value"))
		{
			CommandsListView->GetHeaderRow()->InsertColumn(
				SHeaderRow::Column("Value")
				.DefaultLabel(FText::FromString("Execute Value"))
				,
				1
			);
		}
		
	}

	
	const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentCommandsSharedPtr();

	FText SearchText = SearchBox->GetText();

	if (SearchText.IsEmpty())
	{
		CommandsListView->SetListItemsSource(Commands);
		CommandsListView->RebuildList();
		return;
	}

	FilteredListView.Empty();
	for (const auto& Command : Commands)
	{
		FString Part = SearchText.ToString();
		if (Command->Name.Contains(Part))
		{
			FilteredListView.Add(Command);
		}

	}

	CommandsListView->SetListItemsSource(FilteredListView);
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
		.SelectAllTextWhenFocused(true)
		.HintText(FText::FromString("New name"))
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

	EditDialog->SetWidgetToFocusOnActivate(Widget);

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

bool SConsoleManagerSlateWidget::OpenExecMultipleDialog(TArray<TSharedPtr<FConsoleCommand>> Commands)
{
	TSharedPtr<SWidget> WidgetToFocus;

	TSharedRef<SGridPanel> GridPanel = SNew(SGridPanel);

	for (int i = 0; i < Commands.Num(); i++)
	{
		const auto& SelectedCommand = Commands[i];

		TSharedRef<SEditableTextBox> EditText =
			SNew(SEditableTextBox)
			.Text(FText::FromString(SelectedCommand->CurrentValue))
			.MinDesiredWidth(100);


		if (i == 0)
		{
			WidgetToFocus = EditText;
		}

		GridPanel->AddSlot(0, i)
			.Padding(FMargin(0.0f, 5.0f, 10.0f, 5.f))
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedCommand->Name))
			];

		GridPanel->AddSlot(1, i)
			.Padding(FMargin(0.0f, 5.0f, 0.0f, 5.0f))
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				EditText
			];
	}

	FText ExecuteTitle = FText::FromString("Execute");

	if (Commands.Num() > 1)
	{
		ExecuteTitle = FText::FromString("Execute Mutliple");
	}

	TSharedRef<SCustomDialog> ExecuteCommands = SNew(SCustomDialog)
		.Title(ExecuteTitle)
		.DialogContent(GridPanel)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});

	ExecuteCommands->SetWidgetToFocusOnActivate(WidgetToFocus);

	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = ExecuteCommands->ShowModal();
	if (ButtonPressed == 0)
	{
		return true;
	}

	return false;
}

TSharedPtr<SWidget> SConsoleManagerSlateWidget::GetListViewContextMenu()
{
	

	const FCommandGroup& Group = CommandsManager.Pin()->GetCurrentCommandGroup();

	TArray<TSharedPtr<FConsoleCommand>> SelectedCommands;
	CommandsListView->GetSelectedItems(SelectedCommands);


	// If we right clicked on listview and not any entry
	if (SelectedCommands.Num() == 0 && Group.bIsEditable)
	{

		FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());
		MenuBuilder.BeginSection("Command", LOCTEXT("CommandContextMenu_Header_Command", "Command"));
		{
			MenuBuilder.AddMenuEntry
			(
				LOCTEXT("CommandContextMenu_AddCommand", "Add new"),
				LOCTEXT("CommandContextMenu_AddCommand_Desc", "Adds new command"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([=]() { UE_LOG(LogTemp, Warning, TEXT("Add new command")); }), FCanExecuteAction()),
				NAME_None,
				EUserInterfaceActionType::Button
			);
		}
		MenuBuilder.EndSection();
		

		return MenuBuilder.MakeWidget();
	}
	else if (SelectedCommands.Num() > 0)
	{
		FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());
		{
			MenuBuilder.BeginSection("Command", LOCTEXT("CommandContextMenu_Header_Command", "Command"));
			{

				const FText ExecuteName = SelectedCommands.Num() > 1 ? FText::FromString("Execute All") : FText::FromString("Execute");

				MenuBuilder.AddMenuEntry
				(
					ExecuteName,
					LOCTEXT("CommandContextMenu_Execute_Desc", "Execute all selected commands"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda(
						[=]()
						{

							if (bIsAllCommands)
							{

								bool ShouldExecute = OpenExecMultipleDialog(SelectedCommands);
							}
							else
							{

							}

							for (const auto& SelectedCommand : SelectedCommands)
							{
								CommandsManager.Pin()->ExecuteCommand(SelectedCommand.ToSharedRef().Get());
							}
						}), FCanExecuteAction()),
					NAME_None,
					EUserInterfaceActionType::Button
					);

				if (Group.bIsEditable)
				{
					MenuBuilder.AddMenuEntry
					(
						LOCTEXT("CommandContextMenu_AddCommand", "Add New"),
						LOCTEXT("CommandContextMenu_AddCommand_Desc", "Adds new command"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda([=]() { UE_LOG(LogTemp, Warning, TEXT("Add new command")); }), FCanExecuteAction()),
						NAME_None,
						EUserInterfaceActionType::Button
					);


					FUIAction Action_DuplicateGroup(
						FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::DuplicateGroup, 0),
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
						FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::RemoveGroup, 0),
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



			}
			MenuBuilder.EndSection();

			if (SelectedCommands.Num() > 0)
			{
				MenuBuilder.BeginSection("Group", LOCTEXT("GroupContextMenu_Header_Group", "Group"));

				FNewMenuDelegate Delegate;

				Delegate.BindLambda([=](FMenuBuilder& SubMenuBuilder) {
					FUIAction Action_AddCommandToNewGroup(
						FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::RemoveGroup, 0),
						FCanExecuteAction()
					);

					SubMenuBuilder.AddMenuEntry
					(
						LOCTEXT("AddToGroupContextSubmenu_NewGroup", "New Group"),
						LOCTEXT("AddToGroupContextSubmenu_NewGroup_Desc", "Create new group with selected commands"),
						FSlateIcon(),
						Action_AddCommandToNewGroup,
						NAME_None,
						EUserInterfaceActionType::Button
					);
					SubMenuBuilder.AddSeparator();

					auto& CommandGroups = CommandsManager.Pin()->GetCommandGroups();

					for (int i = 0; i < CommandGroups.Num(); i++)
					{

						SubMenuBuilder.AddMenuEntry
						(
							FText::FromString(CommandGroups[i].Name),
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateLambda([=]() {UE_LOG(LogTemp, Warning, TEXT("Selected %s"), *CommandGroups[i].Name); }), FCanExecuteAction()),
							NAME_None,
							EUserInterfaceActionType::Button
						);
					}

					});

				MenuBuilder.AddSubMenu(FText::FromString("Add To Group"), FText::FromString("Add to existing or create new group"), Delegate);




				MenuBuilder.EndSection();
			}
		}

		return MenuBuilder.MakeWidget();
	}

	return SNullWidget::NullWidget;
}

void SConsoleManagerSlateWidget::HandleNewGroup()
{
	const TArray<FString> Groups = CommandsManager.Pin()->GetGroupList();


	TSharedRef<SEditableTextBox> Widget =
		SNew(SEditableTextBox)
		.HintText(FText::FromString("New name"))
		.Text(FText::GetEmpty());

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
				.Text(FText::FromString(FString::Printf(TEXT("Choose name for new group"))))
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				Widget
			]
		];


	TSharedRef<SCustomDialog> NewGroupDialog = SNew(SCustomDialog)
		.Title(FText(LOCTEXT("NewGroupDialog_Title", "New Group")))
		.DialogContent(ContentWidget)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});

	NewGroupDialog->SetWidgetToFocusOnActivate(Widget);

	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = NewGroupDialog->ShowModal();

	switch (ButtonPressed)
	{
	case 0:
		CommandsManager.Pin()->AddNewGroup(Widget->GetText().ToString());
		break;
	}

	GenerateGroupsScrollBox();
}


TSharedRef<SWidget> SConsoleCommandListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	
	if (ColumnName.IsEqual(FName(TEXT("Command"))))
	{
		return SNew(STextBlock)
				.Text(FText::FromString(Item->Name))
				.ToolTipText(FText::FromString(Item->GetTooltip()))
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);

			
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

		TSharedRef<SEditableText> EditBox =
			SNew(SEditableText)
			.HintText(FText::FromString(Item->CurrentValue))
			.OnIsTypedCharValid(FOnIsTypedCharValid::CreateLambda(
				[=](TCHAR Char) {
					if (Item->Type.Equals("Int"))
					{
						return FString().AppendChar(Char).IsNumeric();
					}
					return true;
				}
			))
			.Text(FText::FromString(Item->CurrentValue))
			.ClearKeyboardFocusOnCommit(false)
			.AllowContextMenu(false)
			.SelectAllTextWhenFocused(true)
			.SelectAllTextOnCommit(true)
			.OnTextCommitted_Lambda(
				[=](const FText& NewText, ETextCommit::Type How) {
					if (How == ETextCommit::Type::OnEnter)
					{
						UE_LOG(LogTemp, Warning, TEXT("Commit: %s"), *NewText.ToString());
					}
					else
					{

						//EditBox->SetText(FText::FromString("elo"));
					}
					UE_LOG(LogTemp, Warning, TEXT("Item command: %s"), *Item->Command);

					
				}
		);

		return EditBox;
		//return SNew(STextBlock).Text(FText::FromString(Item->CurrentValue));
	}

	if (ColumnName.IsEqual(FName(TEXT("Execute"))))
	{
		return SNew(SButton).Text(FText::FromString("Execute"))
			.OnClicked(FOnClicked::CreateLambda([=]() {
			UE_LOG(LogTemp, Warning, TEXT("Value of click:%s"), *Item->Value);

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
