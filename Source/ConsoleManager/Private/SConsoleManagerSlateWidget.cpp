// Fill out your copyright notice in the Description page of Project Settings.


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "ConsoleManagerCommands.h"

#include "Widgets/Layout/SScrollBox.h"

#include "Widgets/Layout/SGridPanel.h"
#include "Dialogs/Dialogs.h"
#include "Dialogs/CustomDialog.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SConsoleManagerSlateWidget::Construct(const FArguments& InArgs)
{

	UE_LOG(LogTemp, Warning, TEXT("Construct slate widget"));
	CommandsManager = InArgs._CommandsManager;

	TSharedPtr< SHeaderRow > HeaderRow = SNew(SHeaderRow);
	

	HeaderRow->AddColumn(SHeaderRow::Column("Command")
		.DefaultLabel(FText::FromString("Command"))
		.FillWidth(2.0f)
		.HAlignCell(EHorizontalAlignment::HAlign_Fill)
		.HAlignHeader(EHorizontalAlignment::HAlign_Fill)
		.VAlignCell(EVerticalAlignment::VAlign_Center));


	HeaderValue.ColumnId("Value");
	HeaderValue.DefaultTooltip(FText::FromString("The value with which the command will be executed"));
	HeaderValue.DefaultLabel(FText::FromString("Execute Value"));
	HeaderValue.HAlignCell(EHorizontalAlignment::HAlign_Fill);
	HeaderValue.HAlignHeader(EHorizontalAlignment::HAlign_Center);
	HeaderValue.VAlignCell(EVerticalAlignment::VAlign_Center);


	if (InArgs._DisplayCommandValueType)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("Type")
			.DefaultLabel(FText::FromString("Type"))
			.DefaultTooltip(FText::FromString("Variable Type"))
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center)
			.FixedWidth(80)
		);

	}
	
	FString SetByDescription = TEXT(
		"Represents where variable was set, description below in order from lowest priority to highest:" LINE_TERMINATOR LINE_TERMINATOR
		"Constructor:			Lowest priority (default after console variable creation)" LINE_TERMINATOR 
		"Scalability:			From Scalability.ini (lower priority than game settings so it's easier to override partially)" LINE_TERMINATOR 
		"GameSetting:			(in game UI or from file)" LINE_TERMINATOR 
		"ProjectSetting:		Project settings (editor UI or from file, higher priority than game setting to allow to enforce some setting fro this project)" LINE_TERMINATOR 
		"SystemSettingsIni:		Per project setting (ini file e.g. Engine.ini or Game.ini)" LINE_TERMINATOR 
		"DeviceProfile:			Per device setting (e.g. specific iOS device, higher priority than per project to do device specific settings)" LINE_TERMINATOR 
		"ConsoleVariablesIni:	Consolevariables.ini (for multiple projects)" LINE_TERMINATOR 
		"Commandline:			A minus command e.g. -VSync (very high priority to enforce the setting for the application)" LINE_TERMINATOR 
		"Code:					Least useful, likely a hack, maybe better to find the correct SetBy..." LINE_TERMINATOR 
		"Console:				Editor UI or console in game or editor" 
	);


	if (InArgs._DisplaySetByValue)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("SetBy")
			.DefaultLabel(FText::FromString("Set By"))
			.DefaultTooltip(FText::FromString(SetByDescription))
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center)
			.FixedWidth(130)
		);

	}
	HeaderRow->AddColumn(SHeaderRow::Column("Current Value")
		.DefaultLabel(FText::FromString("Current Engine Value"))
		.DefaultTooltip(FText::FromString("Value that is currently set in Engine" LINE_TERMINATOR "Modifying it instantly executes command with that value!"))
		.HAlignCell(EHorizontalAlignment::HAlign_Fill)
		.HAlignHeader(EHorizontalAlignment::HAlign_Center)
		.VAlignCell(EVerticalAlignment::VAlign_Center)
	);
		


	CommandsListView = SNew(SListView< TSharedPtr<FConsoleCommand> >)
		.ListItemsSource(&CommandsManager.Pin()->GetCurrentCommandsSharedPtr())
		.ItemHeight(25.0f)
		.OnGenerateRow(this, &SConsoleManagerSlateWidget::OnCommandsRowGenerate)
		.SelectionMode(ESelectionMode::Multi)
		.HeaderRow(HeaderRow)
		.OnContextMenuOpening_Raw(this, &SConsoleManagerSlateWidget::GetListViewContextMenu);
		

	//CommandsListView->SetOnEntryInitialized(SListView<TSharedPtr<FConsoleCommand>>::FOnEntryInitialized::CreateLambda(
	//	[=](TSharedPtr<FConsoleCommand> Item, const TSharedRef<ITableRow> Row) {

	//		
	//		

	//		bool IsGen = CommandsListView->GetHeaderRow()->IsColumnGenerated("Command");
	//		FVector2D MaxSize = Row->GetRowSizeForColumn("Command");
	//		UE_LOG(LogTemp, Warning, TEXT("%s : %f : %f"), *MaxSize.ToString(), CommandsListView->GetHeaderRow()->GetColumns()[0].GetWidth(), MaxSize.X);
	//		//CommandsListView->GetHeaderRow()->SetColumnWidth("Command", MaxSize.X);

	//	}));

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
							FilterString = NewText.ToString();	
							FilterList();
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

						CommandsListView->ScrollToTop();

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
			.Padding(5.0f, 5.0f, 5.0f, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0.0f, 5.0f, 0.0f, 5.0f)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0, 0, 5.0f, 0)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged_Lambda([=](ECheckBoxState State) {

							bShowCVar = (bool)State;

							FilterList();
						})
						.Content()
						[
							SNew(STextBlock)
							.Text(FText::FromString("Show variables"))
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(0, 0, 5.0f, 0)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged_Lambda([=](ECheckBoxState State) {
							bShowCCmd = (bool)State;
							FilterList();
						})
						.Content()
						[
							SNew(STextBlock)
							.Text(FText::FromString("Show commands"))
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(0, 0, 5.0f, 0)
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged_Lambda([=](ECheckBoxState State) {

							bShowExec = (bool)State;
							
							FilterList();

						})
						.Content()
						[
							SNew(STextBlock)
							.Text(FText::FromString("Show exec commands"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(0,0,0, 0.0f)
				[
					CommandsListView.ToSharedRef()
					//CommandsScrollBox.ToSharedRef()
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

void SConsoleManagerSlateWidget::FilterList()
{
	const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentCommandsSharedPtr();

	bool bShowAll = bShowCCmd && bShowCVar && bShowExec;
	if (FilterString.IsEmpty() && bShowAll)
	{
		CommandsListView->SetListItemsSource(Commands);
		CommandsListView->RebuildList();
		return;
	}

	FilteredListView.Empty();
	for (const auto& Command : Commands)
	{
		bool bShouldBeDisplayed = true;
		if (Command->GetObjType() == EConsoleCommandType::CVar)
		{
			bShouldBeDisplayed = bShowCVar;
		}
		else if(Command->GetObjType() == EConsoleCommandType::CCmd)
		{
			bShouldBeDisplayed = bShowCCmd;
		}
		else if(Command->GetObjType() == EConsoleCommandType::Exec)
		{
			bShouldBeDisplayed = bShowExec;
		}

		// If its empty no need to check if string contains empty string
		bool bContainFilterString = FilterString.IsEmpty() || Command->GetName().Contains(FilterString);

		if (bContainFilterString && bShouldBeDisplayed)
		{
			FilteredListView.Add(Command);
		}
	}
	

	CommandsListView->SetListItemsSource(FilteredListView);
	CommandsListView->RebuildList();

}

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


TSharedRef<ITableRow> SConsoleManagerSlateWidget::OnCommandsRowGenerate(TSharedPtr<FConsoleCommand> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	Item->Refresh();

	const FCommandGroup& CurrentGroup = CommandsManager.Pin()->GetCurrentCommandGroup();

	TSharedRef<SConsoleCommandListRow> Row = SNew(SConsoleCommandListRow, OwnerTable).Item(Item).bIsValid(Item->IsValid()).bIsEditable(CurrentGroup.bIsEditable);


	Row->SetOnCommandValueEdit(FOnCommandValueEdit::CreateLambda(
		[=](TSharedPtr<FConsoleCommand> Command, int32 Index)
		{
			CommandsManager.Pin()->ReplaceCommandInCurrentGroup(Index, Command.ToSharedRef().Get());
		}
	));

	Row->SetOnExecuteCommand(FOnExecuteCommand::CreateLambda(
		[=](const FConsoleCommand& Command)
		{
			bNeedsRefresh = false;
			CommandsManager.Pin()->UpdateCurrentEngineValue(Command);
			bNeedsRefresh = true;

		}
	));

	Row->SetOnEngineValueEditing(FOnSimpleCommand::CreateLambda(
		[=](TSharedPtr<FConsoleCommand> Command)
		{
			CommandsListView->SetSelection(Command);
		}
	));

	

	//Do not allow to reorder elements in all commands
	if (!CurrentGroup.bIsEditable)
	{
		return Row;
	}

	Row->SetOnDragDetected(FOnDragDetected::CreateLambda(
		[=](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {

			TSharedRef<DragNDrop> DragOp = MakeShareable(new DragNDrop);

			DragOp->Manager = CommandsManager.Pin();
			DragOp->Command = Item;

			auto t1 = std::chrono::high_resolution_clock::now();
		
			CommandsListView->ClearSelection();
			CommandsListView->SetSelection(Item, ESelectInfo::Direct);

			DragOp->Id = CommandsManager.Pin()->GetCurrentCommandsSharedPtr_Cache().Find(Item);

			auto t2 = std::chrono::high_resolution_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

			
			int64 Time = duration;
			
			UE_LOG(LogTemp, Warning, TEXT("Time taken: %lld"), Time);

			
			return FReply::Handled().BeginDragDrop(DragOp);
		}
	));


	Row->SetOnAcceptDrop(STableRow<TSharedPtr<FConsoleCommand>>::FOnAcceptDrop::CreateLambda(
		[=](const FDragDropEvent& DragDropEvent, EItemDropZone Zone, TSharedPtr<FConsoleCommand> Item)
		{

			TSharedPtr<DragNDrop> DragConnectionOp = DragDropEvent.GetOperationAs<DragNDrop>();

			if (DragConnectionOp.IsValid())
			{
				CommandsManager.Pin()->GetCurrentCommands();

				int32 NewPosition = CommandsManager.Pin()->GetCurrentCommandsSharedPtr_Cache().Find(Item);

				if (Zone != EItemDropZone::AboveItem)
				{
					NewPosition += 1;
				}

				CommandsManager.Pin()->ReorderCommandInCurrentGroup(DragConnectionOp->Id, NewPosition);

				GenerateCommandsScrollBox();

				UE_LOG(LogTemp, Warning, TEXT("Dragged obj: %s with ID: %d ||| Over: %s"), *DragConnectionOp->Command->Name, DragConnectionOp->Id, *Item->Name);
			}

			return FReply::Handled();
		}
	));

	

	return Row;
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
	CommandsListView->ClearSelection();
	if (bIsAllCommands)
	{
		CommandsListView->GetHeaderRow()->RemoveColumn("Value");
	}
	else
	{

		if(!CommandsListView->GetHeaderRow()->IsColumnGenerated("Value"))
		{
			CommandsListView->GetHeaderRow()->InsertColumn(HeaderValue, 1);
		}
		
	}

	
	const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentCommandsSharedPtr();

	FilterList();
	

	

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

	TArray<TSharedPtr<SEditableTextBox>> TextBoxes;
	TextBoxes.Reserve(Commands.Num());

	for (int i = 0; i < Commands.Num(); i++)
	{
		const auto& SelectedCommand = Commands[i];

		TSharedRef<SEditableTextBox> EditText =
			SNew(SEditableTextBox)
			.Text(FText::FromString(SelectedCommand->GetCurrentValue()))
			.MinDesiredWidth(100);

		TextBoxes.Add(EditText);

		if (i == 0)
		{
			WidgetToFocus = EditText;
		}

		GridPanel->AddSlot(0, i)
			.Padding(FMargin(0.0f, 5.0f, 10.0f, 5.f))
			.VAlign(EVerticalAlignment::VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(SelectedCommand->GetName()))
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
		for (int i = 0; i < Commands.Num(); i++)
		{
			Commands[i]->SetValue(TextBoxes[i]->GetText().ToString());
		}

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
				LOCTEXT("CommandContextMenu_AddCommand", "Add New"),
				LOCTEXT("CommandContextMenu_AddCommand_Desc", "Adds new command"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([=]() { HandleNewCommands(); }), FCanExecuteAction()),
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

							bool ShouldExecute = true;

							if (bIsAllCommands)
							{
								ShouldExecute = OpenExecMultipleDialog(SelectedCommands);
							}

							if (ShouldExecute)
							{
								if (SelectedCommands.Num() > 1)
								{
									CommandsManager.Pin()->ExecuteMultipleCommands(SelectedCommands);
								}
								else
								{
									for (const auto& SelectedCommand : SelectedCommands)
									{
										CommandsManager.Pin()->ExecuteCommand(SelectedCommand.ToSharedRef().Get());
									}
								}
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
						FUIAction(FExecuteAction::CreateLambda([=]() {

							HandleNewCommands();

							
							}), FCanExecuteAction()),
						NAME_None,
						EUserInterfaceActionType::Button
					);

					if (SelectedCommands.Num() == 1)
					{
						FUIAction Action_DuplicateCommand(
							FExecuteAction::CreateLambda(
								[=]()
								{
									CommandsManager.Pin()->DuplicateCommand(CommandsManager.Pin()->GetCurrentCommandsSharedPtr_Cache().Find(SelectedCommands[0]));
									GenerateCommandsScrollBox();

									CommandsListView->SetSelection(SelectedCommands[0]);
								}
							),
							FCanExecuteAction()
						);

						MenuBuilder.AddMenuEntry
						(
							LOCTEXT("GroupContextMenu_DuplicateCommand", "Duplicate"),
							LOCTEXT("GroupContextMenu_DuplicateCommand_Desc", "Duplicate command"),
							FSlateIcon(),
							Action_DuplicateCommand,
							NAME_None,
							EUserInterfaceActionType::Button
						);
					}
					
					FUIAction Action_RemoveCommands(
						FExecuteAction::CreateLambda(
							[=]()
							{
								TArray<int32> Ids;

								const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentCommandsSharedPtr_Cache();

								for (int i = 0; i < SelectedCommands.Num(); i++)
								{
									Ids.Add(Commands.Find(SelectedCommands[i]));
									UE_LOG(LogTemp, Warning, TEXT("Id to be removed: %d"), Commands.Find(SelectedCommands[i]));
								}

								CommandsManager.Pin()->RemoveCommands(Ids);

								GenerateCommandsScrollBox();
							}
						),
						FCanExecuteAction()
					);

					MenuBuilder.AddMenuEntry
					(
						LOCTEXT("GroupContextMenu_RemoveCommands", "Remove"),
						LOCTEXT("GroupContextMenu_RemoveCommands_Desc", "Remove commands"),
						FSlateIcon(),
						Action_RemoveCommands,
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

				Delegate.BindLambda([=](FMenuBuilder& SubMenuBuilder) 
				{

					FUIAction Action_AddCommandToNewGroup(
					FExecuteAction::CreateLambda(
						[=]() 
						{
							FCommandGroup* NewGroup = HandleNewGroup();

							if (NewGroup)
							{
								CommandsManager.Pin()->AddCommandsToGroup(NewGroup, SelectedCommands);
							}
						}),
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
							FUIAction(FExecuteAction::CreateLambda(
								[=]() 
								{
									FCommandGroup* FoundGroup = CommandsManager.Pin()->GetGroupById(CommandGroups[i].Id);
									if (FoundGroup)
									{
										CommandsManager.Pin()->AddCommandsToGroup(FoundGroup, SelectedCommands);
										UE_LOG(LogTemp, Warning, TEXT("Add %d commands to %s"), SelectedCommands.Num(), *CommandGroups[i].Name);
									}
									
								}), 
							FCanExecuteAction()),
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

FCommandGroup* SConsoleManagerSlateWidget::HandleNewGroup()
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
	
	FCommandGroup* NewGroup = nullptr;

	switch (ButtonPressed)
	{
	case 0:
		NewGroup = CommandsManager.Pin()->AddNewGroup(Widget->GetText().ToString());
		break;
	}

	GenerateGroupsScrollBox();

	return NewGroup;
}

void SConsoleManagerSlateWidget::HandleNewCommands()
{
	TSharedRef<SMultiLineEditableTextBox> MultiLineEditText =
		SNew(SMultiLineEditableTextBox)
		.HintText(FText::FromString("New commands"))
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
			.Text(FText::FromString(FString::Printf(TEXT("Add new commands, every line is a single command"))))
		.AutoWrapText(true)
		.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
		]
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MultiLineEditText
		]
		];


	TSharedRef<SCustomDialog> NewCommandDialog = SNew(SCustomDialog)
		.Title(FText(LOCTEXT("NewCommandDialog_Title", "New Commands")))
		.DialogContent(ContentWidget)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});

	NewCommandDialog->SetWidgetToFocusOnActivate(MultiLineEditText);

	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = NewCommandDialog->ShowModal();



	switch (ButtonPressed)
	{
	case 0:
		TArray<TSharedPtr<FConsoleCommand>> NewCommands;

		TArray<FString> ParsedCommands;
		MultiLineEditText->GetText().ToString().ParseIntoArrayLines(ParsedCommands);

		for (const auto& StringCommand : ParsedCommands)
		{
			NewCommands.Add(MakeShareable(new FConsoleCommand(StringCommand)));
		}

		CommandsManager.Pin()->AddCommandsToCurrentGroup(NewCommands);
		GenerateCommandsScrollBox();
		CommandsListView->ScrollToBottom();
		break;
	}
}


#include "Widgets/Text/SRichTextBlock.h"

TSharedRef<SWidget> SConsoleCommandListRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	
	if (ColumnName.IsEqual(FName(TEXT("Command"))))
	{
		TSharedPtr<SWidget> ErrorWidget;
		if (!Item->IsValid())
		{
			
			ErrorWidget = SNew(SImage)
				.Image(FCoreStyle::Get().GetBrush("Icons.Error"))
				.ToolTipText(FText::FromString("This command was not found, however it doesnt mean its invalid!"));
		}
		else
		{
			ErrorWidget = SNullWidget::NullWidget;
		}
		


		TSharedRef<SHorizontalBox> CommandWidget =
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ErrorWidget.ToSharedRef()
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->GetName()))
				.ToolTipText(FText::FromString(Item->GetTooltip()))
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
			];
	

		return CommandWidget;
	
	}
	if (ColumnName.IsEqual(FName(TEXT("Value"))))
	{
		TSharedPtr<FConsoleCommand> LocalCmd = Item;

		TAttribute<FSlateColor> BorderColor = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([LocalCmd]() {

			return LocalCmd->GetValue().Equals(LocalCmd->GetCurrentValue()) ? FLinearColor(0, 255,0) : FLinearColor(255,0,0);
		}));

		TAttribute<FText> ValueText = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([LocalCmd]() {
			return FText::FromString(LocalCmd->GetValue());
		}));

		

		TSharedRef<SEditableText> EditText = SNew(SEditableText)
		.HintText(ValueText)
		.OnIsTypedCharValid(FOnIsTypedCharValid::CreateLambda(
			[=](TCHAR Char) {
				if (Item->GetType().Equals("Int") || Item->GetType().Equals("Float"))
				{
					return FString().AppendChar(Char).IsNumeric();
				}
				return true;
			}
		))
		.Text(FText::FromString(Item->GetValue()))
		.ClearKeyboardFocusOnCommit(false)
		.AllowContextMenu(false)
		.SelectAllTextWhenFocused(true)
		.SelectAllTextOnCommit(true)
		.RevertTextOnEscape(true)
		.Justification(ETextJustify::Center)
		.Text(ValueText)
		.OnTextCommitted_Lambda
		(
			[=](const FText& NewText, ETextCommit::Type How) 
			{
				if (How == ETextCommit::Type::OnEnter)
				{
					Item->SetValue(NewText.ToString());

					if (OnCommandValueEdit.IsBound())
					{
						OnCommandValueEdit.Execute(Item, IndexInList);

					}
				}
				else if( How == ETextCommit::OnCleared)
				{
					
					//EditBox->SetText(FText::FromString("elo"));
				}
				UE_LOG(LogTemp, Warning, TEXT("ValueEdited: %s"), *Item->GetExec());

			}
		);


		TSharedRef<SBorder> ValueBorder = SNew(SBorder)
			.BorderBackgroundColor(Item->GetObjType() == EConsoleCommandType::CVar ? BorderColor : FLinearColor(0,0,0,0))
			[
				EditText
			];
		
		return ValueBorder;
	}
	else if (ColumnName.IsEqual(FName(TEXT("Type"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->GetType()));
	}
	else if (ColumnName.IsEqual(FName(TEXT("SetBy"))))
	{
		return SNew(STextBlock).Text(FText::FromString(Item->GetSetBy()));
	}
	else if (ColumnName.IsEqual(FName(TEXT("Current Value"))))
	{

		TSharedPtr<SWidget> CurrentValueCell;
		
		TSharedPtr<FConsoleCommand> LocalCmd = Item;

		TAttribute<FText> CurrentValueText = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([LocalCmd]() {
			return FText::FromString(LocalCmd->GetCurrentValue());
		}));

		FText EditTextValue = FText::FromString(Item->GetCurrentValue());

		TSharedRef<SEditableText> EditText = SNew(SEditableText)
		.HintText(CurrentValueText)
		.OnIsTypedCharValid(FOnIsTypedCharValid::CreateLambda(
			[=](TCHAR Char) {
				if (Item->GetType().Equals("Int") || Item->GetType().Equals("Float"))
				{
					return FString().AppendChar(Char).IsNumeric();
				}
				return true;
			}
		))
		.Text(FText::FromString(Item->GetCurrentValue()))
		.ClearKeyboardFocusOnCommit(false)
		.AllowContextMenu(false)
		.SelectAllTextWhenFocused(true)
		.SelectAllTextOnCommit(true)
		.RevertTextOnEscape(true)
		.Justification(ETextJustify::Center)
		.Text(CurrentValueText)
		.OnTextCommitted_Lambda
		(
			[=](const FText& NewText, ETextCommit::Type How) 
			{
				if (How == ETextCommit::Type::OnEnter)
				{
					FConsoleCommand TempCommand(Item.ToSharedRef().Get());

					TempCommand.SetValue(NewText.ToString());

					if (OnExecuteCommand.IsBound())
					{
						OnExecuteCommand.Execute(TempCommand);

						Item->Refresh();
					}

					UE_LOG(LogTemp, Warning, TEXT("Commit: %s"), *NewText.ToString());
				}
				else if( How == ETextCommit::OnCleared)
				{
					
					//EditBox->SetText(FText::FromString("elo"));
				}
				UE_LOG(LogTemp, Warning, TEXT("Item command: %s"), *Item->GetExec());

			}
		);

		bool bContainsValueColumn_Copy = bContainsValueColumn;
		TAttribute<FSlateColor> BorderColor = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([LocalCmd, bContainsValueColumn_Copy]() {

			if (!bContainsValueColumn_Copy)
			{
				return FLinearColor(0, 255, 0);
			}

			const bool AreValuesEqual = LocalCmd->GetValue().Equals(LocalCmd->GetCurrentValue());
			const FLinearColor RetColor = AreValuesEqual ? FLinearColor(0, 255, 0) : FLinearColor(255, 0, 0);


			return RetColor;
			}));

		CurrentValueCell = SNew(SBorder)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.BorderBackgroundColor(Item->GetObjType() == EConsoleCommandType::CVar ? BorderColor : FLinearColor(0, 0, 0, 0))
			[
				EditText	
			];


		if (Item->GetObjType() != EConsoleCommandType::CVar)
		{
			CurrentValueCell->SetVisibility(EVisibility::Hidden);
		}


		return CurrentValueCell.ToSharedRef();
	}

	return SNullWidget::NullWidget;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
