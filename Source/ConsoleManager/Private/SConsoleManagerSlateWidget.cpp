// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga


#include "SConsoleManagerSlateWidget.h"
#include "SlateOptMacros.h"
#include "ConsoleManagerCommands.h"



#include "Widgets/Layout/SGridPanel.h"
#include "Dialogs/Dialogs.h"
#include "Dialogs/CustomDialog.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "ConsoleManager.h"
#include "CommandsContainer.h"

#include "Framework/MultiBox/SToolBarComboButtonBlock.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"

#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "FConsoleManagerModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SConsoleManagerSlateWidget::Construct(const FArguments& InArgs)
{
	CommandsManager = InArgs._CommandsManager;

	CommandsManager.Pin()->OnDataRefreshed.BindLambda(
		[=]() {

			GenerateGroupsScrollBox();
			GenerateCommandsScrollBox();
			UE_LOG(LogTemp, Warning, TEXT("DATA NEEDS REFRESH!"));
		}
	);

	TSharedPtr< SHeaderRow > HeaderRow = SNew(SHeaderRow);

	HeaderRow->AddColumn(SHeaderRow::Column("Command")
		.DefaultLabel(FText::FromString("Command"))
		.FillWidth(2.0f)
		.HAlignCell(EHorizontalAlignment::HAlign_Fill)
		.HAlignHeader(EHorizontalAlignment::HAlign_Fill)
		.VAlignCell(EVerticalAlignment::VAlign_Center));

	HeaderExec.ColumnId("Execute");
	HeaderExec.DefaultLabel(FText::FromString(""));
	HeaderExec.FixedWidth(40.0f);
	HeaderExec.HAlignCell(EHorizontalAlignment::HAlign_Center);
	HeaderExec.HAlignHeader(EHorizontalAlignment::HAlign_Fill);
	HeaderExec.VAlignCell(EVerticalAlignment::VAlign_Center);


	HeaderValue.ColumnId("Value");
	HeaderValue.DefaultTooltip(FText::FromString("The value with which the command will be executed"));
	HeaderValue.DefaultLabel(FText::FromString("Preset Value"));
	HeaderValue.HAlignCell(EHorizontalAlignment::HAlign_Fill);
	HeaderValue.HAlignHeader(EHorizontalAlignment::HAlign_Center);
	HeaderValue.VAlignCell(EVerticalAlignment::VAlign_Center);
	
	if (InArgs._DisplayCommandType)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("CommandType")
			.DefaultLabel(FText::FromString("Type"))
			.ManualWidth(60.0f)
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center));

	}
	

	if (InArgs._DisplayCommandValueType)
	{
		HeaderRow->AddColumn(SHeaderRow::Column("Type")
			.DefaultLabel(FText::FromString("Param Type"))
			.DefaultTooltip(FText::FromString("Variable Type"))
			.HAlignCell(EHorizontalAlignment::HAlign_Center)
			.HAlignHeader(EHorizontalAlignment::HAlign_Center)
			.VAlignCell(EVerticalAlignment::VAlign_Center)
			.ManualWidth(80)
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
			.ManualWidth(130)
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
		.ListItemsSource(&CommandsManager.Pin()->GetCurrentSharedCommands())
		.ItemHeight(25.0f)
		.OnGenerateRow(this, &SConsoleManagerSlateWidget::OnCommandsRowGenerate)
		.SelectionMode(ESelectionMode::Multi)
		.HeaderRow(HeaderRow)
		.OnContextMenuOpening_Raw(this, &SConsoleManagerSlateWidget::GetListViewContextMenu);


	//CommandsListView->SetOnEntryInitialized(SListView<TSharedPtr<FConsoleCommand>>::FOnEntryInitialized::CreateLambda(
	//	[=](TSharedPtr<FConsoleCommand> Item, const TSharedRef<ITableRow> Row) {

	//		

	//		//bool IsGen = CommandsListView->GetHeaderRow()->IsColumnGenerated("Command");
	//		//FVector2D MaxSize = Row->GetRowSizeForColumn("Command");
	//		//UE_LOG(LogTemp, Warning, TEXT("%s : %f : %f"), *MaxSize.ToString(), CommandsListView->GetHeaderRow()->GetColumns()[0].GetWidth(), MaxSize.X);
	//		////CommandsListView->GetHeaderRow()->SetColumnWidth("Command", MaxSize.X);

	//	}));

	GroupsScrollBox = SNew(SScrollBox);

	CommandsScrollBox = SNew(SScrollBox);

	//FConsoleManagerCommands::Get().GroupContextMenu = MakeShareable(new FUICommandList);


	//FPointerEventHandler GroupsScrollBoxRightClick;

	//GroupsScrollBoxRightClick.BindLambda(
	//	[=](const FGeometry& Geometry, const FPointerEvent& MouseEvent)
	//	{
	//		FMenuBuilder MenuBuilder(true, NULL, TSharedPtr<FExtender>());

	//		{
	//			MenuBuilder.BeginSection("Group", LOCTEXT("GroupContextMenu_Header_Group", "Group"));
	//			{

	//				MenuBuilder.AddMenuEntry
	//				(
	//					LOCTEXT("GroupContextMenu_NewGroup", "Add New"),
	//					LOCTEXT("GroupContextMenu_NewGroup_Desc", "Add new group"),
	//					FSlateIcon(),
	//					FUIAction(FExecuteAction::CreateLambda(
	//						[=]() {
	//							HandleNewGroup();
	//							

	//						}),
	//						FCanExecuteAction()),
	//					NAME_None,
	//					EUserInterfaceActionType::Button
	//				);
	//			}
	//			MenuBuilder.EndSection();
	//		}

	//		if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	//		{
	//			TSharedRef<SWidget> MenuContents = MenuBuilder.MakeWidget();
	//			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	//			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuContents, MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	//			return FReply::Handled();
	//		}
	//	
	//		return FReply::Unhandled();
	//		
	//	});

	//Validate all console commands to check if any existing in AllCommands //git
	TSharedRef<SWidget> Content = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5.0f)
		[// Left Panel
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.Padding(FMargin(2.0f, 0.f))
				.AutoWidth()
				[
					GetMenuButton(LOCTEXT("SnapshotButton", "Snapshot"), FConsoleManagerStyle::Get().GetBrush("Icons.Snapshot"), 
					FOnClicked::CreateLambda([=]() {
							FString NewGroupName;
							UCommandsContainer* SelectedContainer;
							if (HandleNewGroup(NewGroupName, SelectedContainer))
							{
								CommandsManager.Pin()->CreateSnapshotCVars(NewGroupName, SelectedContainer);
							}

							return FReply::Handled();
						}))
				]

				+ SHorizontalBox::Slot()
				.Padding(FMargin(2.0f, 0.f))
				.AutoWidth()
				[

					GetMenuButton(LOCTEXT("RevertButton", "Revert"), FConsoleManagerStyle::Get().GetBrush("Icons.Revert"),
					FOnClicked::CreateLambda([=]() {
							/*FString NewGroupName;
							UCommandsContainer* SelectedContainer;
							if (HandleNewGroup(NewGroupName, SelectedContainer))
							{
								CommandsManager.Pin()->CreateSnapshotCVars(NewGroupName, SelectedContainer);
							}*/

							return FReply::Handled();
						}))
				]

				+ SHorizontalBox::Slot()
				.Padding(FMargin(2.0f, 0.f))
				.AutoWidth()
				[

					GetMenuButton(LOCTEXT("SaveButton", "Save"), FConsoleManagerStyle::Get().GetBrush("Icons.Save"),
					FOnClicked::CreateLambda([=]() {
							/*FString NewGroupName;
							UCommandsContainer* SelectedContainer;
							if (HandleNewGroup(NewGroupName, SelectedContainer))
							{
								CommandsManager.Pin()->CreateSnapshotCVars(NewGroupName, SelectedContainer);
							}*/

							return FReply::Handled();
						}))

				]
			]


			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(34.0f)
				[
					SNew(SCheckBox)
					.Type(ESlateCheckBoxType::ToggleButton)
					.ForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
					.Style(&FConsoleManagerStyle::Get().GetWidgetStyle<FCheckBoxStyle>("GlobalPresetToggleButton"))
					.Content()
					[
						SNew(SOverlay)
						//+ SOverlay::Slot()
						//.HAlign(EHorizontalAlignment::HAlign_Center)
						//.VAlign(EVerticalAlignment::VAlign_Center)
						//[
						//	SNew(SImage)
						//	.Image(FConsoleManagerStyle::Get().GetBrush("Image.GlobalPresetButton"))
						//]
						+ SOverlay::Slot()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Top)
						.Padding(0.f, 7.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("HistoryButton", "History"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.Font(FConsoleManagerStyle::Get().GetFontStyle("GlobalPresetButtonFont"))
						]

					]
					.IsChecked(this, &SConsoleManagerSlateWidget::GetCurrentSelectedGroup, CommandsManager.Pin()->GetHistory()->Id)
					.OnCheckStateChanged_Lambda([=](ECheckBoxState NewRadioState) {
						if (CommandsManager.Pin()->SetActiveHistory())
						{
							GenerateCommandsScrollBox();
							CommandsListView->ScrollToBottom();
						}
					})
				]
				/*SNew(SButton)
				.ButtonStyle(&FConsoleManagerStyle::Get().GetWidgetStyle<FButtonStyle>("EmptyButton"))
				.ForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
				.Content()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.VAlign(EVerticalAlignment::VAlign_Center)
					[
						SNew(SImage)
						.Image(FConsoleManagerStyle::Get().GetBrush("Image.GlobalPresetButton"))
					]
					+ SOverlay::Slot()
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.VAlign(EVerticalAlignment::VAlign_Top)
					.Padding(0.f, 7.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HistoryButton", "History"))
						.TransformPolicy(ETextTransformPolicy::ToUpper)
						.Font(FConsoleManagerStyle::Get().GetFontStyle("GlobalPresetButtonFont"))
					]
					
				]
				.OnClicked(FOnClicked::CreateLambda([=]() {
					if(CommandsManager.Pin()->SetActiveHistory())
					{
						GenerateCommandsScrollBox();
						CommandsListView->ScrollToBottom();
					}
						
					return FReply::Handled();
				}))*/
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(34.0f)
				[
					SNew(SCheckBox)
					.Type(ESlateCheckBoxType::ToggleButton)
					.ForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
					.Style(&FConsoleManagerStyle::Get().GetWidgetStyle<FCheckBoxStyle>("GlobalPresetToggleButton"))
					.Content()
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Top)
						.Padding(0.f, 7.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AllCommandsButton", "All Commands"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.Font(FConsoleManagerStyle::Get().GetFontStyle("GlobalPresetButtonFont"))
						]

					]
					.IsChecked(this, &SConsoleManagerSlateWidget::GetCurrentSelectedGroup, CommandsManager.Pin()->GetAllCommands()->Id)
					.OnCheckStateChanged_Lambda([=](ECheckBoxState NewRadioState) {
						if (CommandsManager.Pin()->SetActiveAllCommands())
						{
							GenerateCommandsScrollBox();

							CommandsListView->ScrollToTop();
						}
					})
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(34.0f)
				[
					SNew(SCheckBox)
					.Type(ESlateCheckBoxType::ToggleButton)
					.ForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
					.Style(&FConsoleManagerStyle::Get().GetWidgetStyle<FCheckBoxStyle>("GlobalPresetToggleButton"))
					.Content()
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Top)
						.Padding(0.f, 7.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("SnapshotButton", "Snapshot"))
							.TransformPolicy(ETextTransformPolicy::ToUpper)
							.Font(FConsoleManagerStyle::Get().GetFontStyle("GlobalPresetButtonFont"))
							]

							]
							.IsChecked(this, &SConsoleManagerSlateWidget::GetCurrentSelectedGroup, CommandsManager.Pin()->GetSnapshot()->Id)
							.OnCheckStateChanged_Lambda([=](ECheckBoxState NewRadioState) {
								if (CommandsManager.Pin()->SetActiveSnapshot())
								{
									GenerateCommandsScrollBox();

									CommandsListView->ScrollToTop();
								}
						})
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(FVector2D(0.0f, 10.0f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PresetsHeader", "Presets"))
				.TransformPolicy(ETextTransformPolicy::ToUpper)
				.Justification(ETextJustify::Center)
				.Font(FConsoleManagerStyle::Get().GetFontStyle("GlobalPresetButtonFont"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSeparator)
				.Thickness(3.0f)
			]

			+ SVerticalBox::Slot()
			.FillHeight(5.0f)
			[
				SNew(SBorder)
				.Visibility(EVisibility::SelfHitTestInvisible)

				//.OnMouseButtonUp(GroupsScrollBoxRightClick)
				[
					GroupsScrollBox.ToSharedRef()
				]
					
			]
		]// Left Panel
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(5.0f, 5.0f, 5.0f, 0)
		[// Right Panel
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
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
					
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.Padding(0, 0, 5.0f, 0)
						.AutoWidth()
						[
							SAssignNew(ShowVarsCb, SCheckBox)
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
							SAssignNew(ShowCmdsCb, SCheckBox)
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
							SAssignNew(ShowExecsCb, SCheckBox)
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
						+ SHorizontalBox::Slot()
							.Padding(0, 0, 5.0f, 0)
							.AutoWidth()
							[
								SNew(SCheckBox)
								.IsChecked(ECheckBoxState::Unchecked)
								.OnCheckStateChanged_Lambda([=](ECheckBoxState State) {

									bShowOnlyModified = (bool)State;

									if (bShowOnlyModified)
									{
										ShowVarsCb->SetEnabled(false);
										ShowCmdsCb->SetEnabled(false);
										ShowExecsCb->SetEnabled(false);
									}
									else
									{
										ShowVarsCb->SetEnabled(true);
										ShowCmdsCb->SetEnabled(true);
										ShowExecsCb->SetEnabled(true);
									}

									FilterList();
								})
							.Content()
							[
								SNew(STextBlock)
								.Text(FText::FromString("Show only modified"))
							.ToolTipText(FText::FromString("When checked shows only modified variables (commands and exec's dont have state)"))
							]
						]
					]
				]
					
				+ SHorizontalBox::Slot()
				.Padding(5.0f, 0.f, 0.f, 0.f)
				.AutoWidth()
				[
					GetMenuButton(LOCTEXT("SettingsButton", "Settings"), FConsoleManagerStyle::Get().GetBrush("Icons.Settings"), 
					FOnClicked::CreateLambda([]() {
							FConsoleManagerModule::GetModule().OpenSettings();

							return FReply::Handled();
						})
					)
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1)
			.Padding(0,0,0, 0.0f)
			[
				CommandsListView.ToSharedRef()
				//CommandsScrollBox.ToSharedRef()
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
	const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentSharedCommands();

	// We could iterate directly over commands instead of pointers for faster access
	//const TArray<FConsoleCommand>& CommandsForIterate = CommandsManager.Pin()->GetCurrentCommands();

	bool bShowAll = bShowCCmd && bShowCVar && bShowExec && !bShowOnlyModified;
	if (FilterString.IsEmpty() && bShowAll)
	{
		CommandsListView->SetListItemsSource(Commands);
		CommandsListView->RebuildList();
	}
	else
	{
		FilteredListView.Empty();
		for (const auto& Command : Commands)
		{
			bool bShouldBeDisplayed = true;
			if (Command->GetObjType() == EConsoleCommandType::CVar)
			{
				bShouldBeDisplayed = bShowCVar;
			}
			else if (Command->GetObjType() == EConsoleCommandType::CCmd)
			{
				// CCmd's cannot be modified in any way so skip these if bShowOnlyModified is true
				bShouldBeDisplayed = bShowCCmd && !bShowOnlyModified;
			}
			else if (Command->GetObjType() == EConsoleCommandType::Exec)
			{
				// Exec's cannot be modified in any way so skip these if bShowOnlyModified is true
				bShouldBeDisplayed = bShowExec && !bShowOnlyModified;
			}

			// If its empty no need to check if string contains empty string
			bool bContainFilterString = FilterString.IsEmpty() || Command->GetName().Contains(FilterString);

			bool bIsModified = false;
			if (bShowOnlyModified)
			{
				Command->Refresh();
				bIsModified = !Command->GetValue().Equals(Command->GetCurrentValue(), ESearchCase::IgnoreCase);
			}

			bShouldBeDisplayed = (bShowOnlyModified == bIsModified) && bShouldBeDisplayed;
			
			if (bContainFilterString && bShouldBeDisplayed)
			{
				FilteredListView.Add(Command);
			}
		}

		CommandsListView->SetListItemsSource(FilteredListView);
		CommandsListView->RebuildList();
	}
}

void SConsoleManagerSlateWidget::OnAddGroupButtonClicked()
{
	//CommandsManager.Pin()->AddNewGroup("NewGroup");

	//GenerateGroupsScrollBox();

}

FReply SConsoleManagerSlateWidget::OnSelectGroupClicked(FGuid Id)
{
	if (CommandsManager.Pin()->SetActiveGroup(Id))
	{
		SearchBox->SetText(FText::GetEmpty());
		GenerateCommandsScrollBox();
	}

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
			//int32 IndexInList;
			//TSharedPtr<ITableRow> Widget = CommandsListView->WidgetFromItem(Command);
			//if (Widget.IsValid())
			//{
			//	TSharedPtr<SConsoleCommandListRow> NewRow = StaticCastSharedPtr<SConsoleCommandListRow>(Widget);

			//	if (NewRow.IsValid())
			//	{
			//		IndexInList = NewRow->GetIndexInList();
			//	}
			//}

			//FChildren* Children = CommandsListView->GetAllChildren();

			//for (int i = 0; i < Children->Num(); i++)
			//{
			//	Children->GetChildAt(i)->
			//}

			//FChildren* Children = CommandsListView->

			//for (int i = 0; i < Children->Num(); i++)
			//{
				//CommandsListView->ItemFromWidget(TSharedPtr<SWidget>(Children->GetChildAt(i)).Get());
				//UE_LOG(LogTemp, Warning, TEXT("%s"), *Children->GetChildAt(i)->GetType().ToString());
				
			//}

			//bNeedsRefresh = false;
			CommandsManager.Pin()->UpdateCurrentEngineValue(Command.ToSharedRef().Get());
			//bNeedsRefresh = true;

		}
	));

	

	//Do not allow to reorder elements in all commands nor history
	if (!CurrentGroup.bIsEditable)
	{
		return Row;
	}

	Row->SetOnDragDetected(FOnDragDetected::CreateLambda(
		[=](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {

			TSharedRef<DragNDrop> DragOp = MakeShareable(new DragNDrop);

			DragOp->Manager = CommandsManager.Pin();
			DragOp->Command = Item;

			//auto t1 = std::chrono::high_resolution_clock::now();
		
			CommandsListView->ClearSelection();
			CommandsListView->SetSelection(Item, ESelectInfo::Direct);

			DragOp->Id = CommandsManager.Pin()->GetCurrentSharedCommands().Find(Item);

			//auto t2 = std::chrono::high_resolution_clock::now();

			//auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
			//
			//int64 Time = duration;
			//
			//UE_LOG(LogTemp, Warning, TEXT("Time taken: %lld"), Time);

			
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

				int32 NewPosition = CommandsManager.Pin()->GetCurrentSharedCommands().Find(Item);

				if (Zone != EItemDropZone::AboveItem)
				{
					NewPosition += 1;
				}

				CommandsManager.Pin()->ReorderCommandInCurrentGroup(DragConnectionOp->Id, NewPosition);

				GenerateCommandsScrollBox();
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


	const TArray<UCommandsContainer*>& Containers = CommandsManager.Pin()->GetCommandsContainers();

	for (int i = 0; i < Containers.Num(); i++)
	{
		UCommandsContainer* Container = Containers[i];
		// In case object was destroyed skip it
		if (!Container->IsValidLowLevel())
		{
			continue;
		}


		TSharedRef<SVerticalBox> ContainerVBox = SNew(SVerticalBox);

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
						FText::FromString("Add new group to " + Container->GetName()),
						LOCTEXT("GroupContextMenu_NewGroup_Desc", "Add new group"),
						FSlateIcon(),
						FUIAction(FExecuteAction::CreateLambda(
							[=]() {

								FString NewGroupName;
								UCommandsContainer* SelectedContainer = nullptr;
								if (HandleNewGroup(NewGroupName, SelectedContainer, Container))
								{
									CommandsManager.Pin()->CreateNewGroup(NewGroupName, SelectedContainer);
								}
									

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




		GroupsScrollBox->AddSlot()
		[
			SNew(SBorder)
			.Padding(FMargin(5, 5, 5, 5))
			.OnMouseButtonUp(GroupsScrollBoxRightClick)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Container->GetName()))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(15.f, .0f, .0f, .0f)
				[
					ContainerVBox
				]
			]

		];
			
		const TArray<FCommandGroup>& Groups = Containers[i]->Groups;

		for (int j = 0; j < Groups.Num(); j++)
		{
			
			const FCommandGroup& Group = Groups[j];

			TAttribute<FText> GroupName = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([Group]() {
				return FText::FromString(Group.Name);
				}));

			TSharedRef<SGroupButton> Button =
				SNew(SGroupButton)
				//.Text(GroupName)
				.OnClicked(this, &SConsoleManagerSlateWidget::OnSelectGroupClicked, Group.Id)
				.Content()
				[
					SNew(STextBlock)
					.Text(GroupName)
				.AutoWrapText(true)
				.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				];

			Button->ShiftRightClickDelegate.BindLambda(
				[this, Group]()
				{
					CommandsManager.Pin()->ExecuteGroup(Group.Id);
				}
			);

			Button->RightClickDelegate.BindLambda(
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
									[this, Container]() {
										FString NewGroupName;
										UCommandsContainer* SelectedContainer = nullptr;

										if (HandleNewGroup(NewGroupName, SelectedContainer, Container))
										{
											CommandsManager.Pin()->CreateNewGroup(NewGroupName, SelectedContainer);
										}
									}),
									FCanExecuteAction()),
								NAME_None,
								EUserInterfaceActionType::Button
							);

							FUIAction Action_EditGroup(
								FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::EditGroup, Group.Id),
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
								FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::DuplicateGroup, Group.Id),
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
								FExecuteAction::CreateRaw(this, &SConsoleManagerSlateWidget::RemoveGroup, Group.Id),
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

			ContainerVBox->AddSlot()
				[
					Button
				];
		}
	}
}

void SConsoleManagerSlateWidget::GenerateCommandsScrollBox()
{
	CommandsListView->ClearSelection();
	if (CommandsManager.Pin()->GetCurrentCommandGroup().Type == EGroupType::AllCommands)
	{
		CommandsListView->GetHeaderRow()->RemoveColumn("Value");
		CommandsListView->GetHeaderRow()->RemoveColumn("Execute");

	}
	else
	{
		if(!CommandsListView->GetHeaderRow()->IsColumnGenerated("Value"))
		{
			CommandsListView->GetHeaderRow()->InsertColumn(HeaderExec, 1);
			CommandsListView->GetHeaderRow()->InsertColumn(HeaderValue, 2);
		}
		
	}

	FilterList();
}

void SConsoleManagerSlateWidget::RemoveGroup(FGuid Id)
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

void SConsoleManagerSlateWidget::DuplicateGroup(FGuid Id)
{
	CommandsManager.Pin()->DuplicateGroup(Id);
	GenerateGroupsScrollBox();
}

void SConsoleManagerSlateWidget::EditGroup(FGuid Id)
{
	const FCommandGroup* Group = CommandsManager.Pin()->GetGroupById(Id);

	TSharedRef<SEditableTextBox> Widget =
		SNew(SEditableTextBox)
		.SelectAllTextWhenFocused(true)
		.HintText(FText::FromString("New name"))
		.Text(FText::FromString(Group->Name));

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
				.Text(FText::FromString(FString::Printf(TEXT("Choose new name for group %s"), *Group->Name)))
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
			.SelectAllTextWhenFocused(true)
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

	FText ExecuteTitle = FText::FromString("Execute Selected");

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

bool SConsoleManagerSlateWidget::DisplayExecuteWarning(const FText& Text)
{
	TSharedRef<SWidget> ContentWidget = SNew(STextBlock)
		.Text(Text);

	TSharedRef<SCustomDialog> NewGroupDialog = SNew(SCustomDialog)
		.Title(FText(LOCTEXT("ExecuteDialog_Title", "Execution warning")))
		.DialogContent(ContentWidget)
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("OK", "OK"), FSimpleDelegate()),
			SCustomDialog::FButton(LOCTEXT("Cancel", "Cancel"), FSimpleDelegate())
			});


	// returns 0 when OK is pressed, 1 when Cancel is pressed, -1 if the window is closed
	const int ButtonPressed = NewGroupDialog->ShowModal();

	switch (ButtonPressed)
	{
	case 0:
		return true;
		break;
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

				const FText ExecuteName = SelectedCommands.Num() > 1 ? FText::FromString("Execute Selected") : FText::FromString("Execute");
				
				EGroupType GroupType = Group.Type;
				MenuBuilder.AddMenuEntry
				(
					ExecuteName,
					LOCTEXT("CommandContextMenu_Execute_Desc", "Execute all selected commands"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda(
						[=]()
						{
							for (auto& Command : SelectedCommands)
							{
								UE_LOG(LogTemp, Warning, TEXT("Command: %s || %s"), *Command->GetName(), *Command->GetValue());
							}

							bool ShouldExecute = true;

							if (GroupType == EGroupType::AllCommands)
							{
								ShouldExecute = OpenExecMultipleDialog(SelectedCommands);
							}

							if (ShouldExecute)
							{
								if (SelectedCommands.Num() > 1)
								{
									//if(DisplayExecuteWarning(FText::FromString("))
									bool bOnlyCVars = true;
									for (auto& Command : SelectedCommands)
									{
										bOnlyCVars = !(Command->GetObjType() == EConsoleCommandType::CCmd || Command->GetObjType() == EConsoleCommandType::Exec);
										if (!bOnlyCVars)
										{
											break;
										}
									}

									if (bOnlyCVars)
									{
										CommandsManager.Pin()->ExecuteMultipleCommands(SelectedCommands);
									}
									else
									{
										const FText WarningText = FText::FromString("Selected rows contain commands and/or exec's, executing multiple of these may lead to crash!\nContinue?");
										if (DisplayExecuteWarning(WarningText))
										{
											CommandsManager.Pin()->ExecuteMultipleCommands(SelectedCommands);
										}
									}
								}
								else
								{
									CommandsManager.Pin()->ExecuteCommand(SelectedCommands[0].ToSharedRef().Get());
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
									CommandsManager.Pin()->DuplicateCommand(CommandsManager.Pin()->GetCurrentSharedCommands().Find(SelectedCommands[0]));
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

								const TArray<TSharedPtr<FConsoleCommand>>& Commands = CommandsManager.Pin()->GetCurrentSharedCommands();

								for (int i = 0; i < SelectedCommands.Num(); i++)
								{
									Ids.Add(Commands.Find(SelectedCommands[i]));
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
							FString NewGroupName;
							UCommandsContainer* SelectedContainer = nullptr;
							if (HandleNewGroup(NewGroupName, SelectedContainer))
							{
								CommandsManager.Pin()->CreateNewGroup(NewGroupName, SelectedContainer, SelectedCommands);
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

					auto& CommandGroups = CommandsManager.Pin()->GetGroupList();

					// Add entry for every group to submenu
					for (int i = 0; i < CommandGroups.Num(); i++)
					{
						SubMenuBuilder.AddMenuEntry
						(
							FText::FromString(CommandGroups[i].Key),
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateLambda(
								[=]() 
								{
									CommandsManager.Pin()->AddCommandsToGroup(CommandGroups[i].Value, SelectedCommands);
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

bool SConsoleManagerSlateWidget::HandleNewGroup(FString& OutName, UCommandsContainer*& OutContainer, UCommandsContainer* InContainer)
{
	TSharedRef<SEditableTextBox> Widget =
		SNew(SEditableTextBox)
		.HintText(FText::FromString("New name"))
		.Text(FText::GetEmpty());

	const TArray<UCommandsContainer*>& Containers = CommandsManager.Pin()->GetCommandsContainers();

	UCommandsContainer* SelectedContainer = InContainer;

	if (!InContainer->IsValidLowLevel() && Containers.Num() > 0)
	{
		SelectedContainer = Containers[0];
	}

	TAttribute<FText> SelectedOption = TAttribute<FText>::Create([&SelectedContainer]() {
		return SelectedContainer->IsValidLowLevel() ? FText::FromString(SelectedContainer->GetName()) : FText::GetEmpty();
		});

	TSharedRef<SComboBox<UCommandsContainer*>> ContainerComboBox = SNew(SComboBox<UCommandsContainer*>)
	.OptionsSource(&Containers)
	.OnGenerateWidget_Lambda([=](UCommandsContainer* ContainerToGenerate) {
		return SNew(STextBlock)
			.Text(FText::FromString(ContainerToGenerate->GetName()));
		}
	)
	.InitiallySelectedItem(InContainer)
	.Content()
	[
		SNew(STextBlock)
		.Text(SelectedOption)
	]
	.OnSelectionChanged_Lambda([&SelectedContainer](UCommandsContainer* NewSelectedContainer, ESelectInfo::Type Info) {
		SelectedContainer = NewSelectedContainer;
	});

	

	TSharedRef<SWidget> ContentWidget =
		SNew(SBox)
		.Padding(FMargin(0.f, 15.f, 0.f, 0.f))
		.WidthOverride(250)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 2))
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
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 10, 0, 2))
			[
				SNew(STextBlock)
				.Text(FText::FromString("Select container"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				ContainerComboBox
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
		OutName = Widget->GetText().ToString();
		OutContainer = ContainerComboBox->GetSelectedItem();
		return true;
		break;
	}

	return false;
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

TSharedRef<SButton> SConsoleManagerSlateWidget::GetMenuButton(FText Text, const FSlateBrush* ImageBrush, FOnClicked ClickedDelegate)
{
	return SNew(SButton)
		.ButtonStyle(&FConsoleManagerStyle::Get().GetWidgetStyle<FButtonStyle>("MenuButton"))
		.ForegroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
		.ContentPadding(2.0f)
		.OnClicked(ClickedDelegate)
		.Content()
		[
			SNew(SVerticalBox)
			// Icon image
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(3.0f)
			.HAlign(HAlign_Center)	// Center the icon horizontally, so that large labels don't stretch out the artwork
			[
				SNew(SImage)
				.Image(ImageBrush)
			]

			// Label text
			+ SVerticalBox::Slot().AutoHeight()
			.Padding(FMargin(1.0f))
			.HAlign(HAlign_Center)	// Center the label text horizontally
			[
				SNew(STextBlock)
				.Text(Text)
				.TextStyle(FCoreStyle::Get(), "ToolBar.Label")	// Smaller font for tool tip labels
				.ShadowOffset(FVector2D::UnitVector)
			]
		];
}

ECheckBoxState SConsoleManagerSlateWidget::GetCurrentSelectedGroup(FGuid Id) const
{
	return CommandsManager.Pin()->GetCurrentCommandGroup().Id == Id ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
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
	else if (ColumnName.IsEqual(FName(TEXT("Execute"))))
	{
		TSharedRef<SWidget> ExecButton = SNew(SBox)
			.Padding(FMargin(1.0f, 1.0f, 1.0f, 0.0f))
			.MaxDesiredWidth(20)
			.MaxDesiredHeight(20)
			[
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(0)
				.Content()
				[
					SNew(SImage)
					.Image(FConsoleManagerStyle::Get().GetBrush("ConsoleManager.ExecAction"))
					.ToolTipText(FText::FromString("Execute command"))
				]
				.OnClicked_Lambda(
					[=]() 
					{
						if (OnExecuteCommand.IsBound())
						{
							OnExecuteCommand.Execute(Item.ToSharedRef().Get());

							Item->Refresh();
						}

						return FReply::Handled();
					}
				)
			];

		return ExecButton;
	} 
	else if (ColumnName.IsEqual(FName(TEXT("Value"))))
	{
		TSharedPtr<FConsoleCommand> LocalCmd = Item;

		TAttribute<FSlateColor> BorderColor = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([LocalCmd]() {

			return LocalCmd->GetValue().Equals(LocalCmd->GetCurrentValue()) ? FConsoleManagerStyle::Get().GetSlateColor("MatchingValues") : FConsoleManagerStyle::Get().GetSlateColor("NotMatchingValues");
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
		.IsEnabled(bIsEditable)
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
				}

			}
		);


		TSharedRef<SBorder> ValueBorder = SNew(SBorder)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
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

					if (OnEngineValueEditing.IsBound())
					{
						OnEngineValueEditing.Execute(TSharedPtr<FConsoleCommand>(new FConsoleCommand(TempCommand)));
						Item->Refresh();
					}

					/*if (OnExecuteCommand.IsBound())
					{
						OnExecuteCommand.Execute(TempCommand);

						Item->Refresh();
					}*/
				}
				else if( How == ETextCommit::OnCleared)
				{
					
					//EditBox->SetText(FText::FromString("elo"));
				}

			}
		);

		CurrentValueEditText = EditText;


		const bool bContainsValueColumn_Copy = bContainsValueColumn;
		TAttribute<FSlateColor> BorderColor = TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateLambda([LocalCmd, bContainsValueColumn_Copy]() {

			if (!bContainsValueColumn_Copy)
			{
				return FConsoleManagerStyle::Get().GetSlateColor("MatchingValues");
			}

			const bool AreValuesEqual = LocalCmd->GetValue().Equals(LocalCmd->GetCurrentValue());
			const FSlateColor RetColor = AreValuesEqual ? FConsoleManagerStyle::Get().GetSlateColor("MatchingValues") : FConsoleManagerStyle::Get().GetSlateColor("NotMatchingValues");


			return RetColor;
		}));

		CurrentValueCell = SNew(SBorder)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.VAlign(EVerticalAlignment::VAlign_Fill)
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
	else if (ColumnName.IsEqual(FName(TEXT("CommandType"))))
	{
		FString Type;
		if (Item->GetObjType() == EConsoleCommandType::CCmd)
		{
			Type = "CCmd";
		}
		else if (Item->GetObjType() == EConsoleCommandType::CVar)
		{
			Type = "CVar";
		}
		else if (Item->GetObjType() == EConsoleCommandType::Exec)
		{
			Type = "Exec";
		}

		return SNew(STextBlock)
			.Text(FText::FromString(Type));
	}



	return SNullWidget::NullWidget;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
