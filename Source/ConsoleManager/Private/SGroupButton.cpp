// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga


#include "SGroupButton.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGroupButton::Construct(const FArguments& InArgs) 
{
	SButton::Construct(InArgs);
	
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
}



FReply SGroupButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = FReply::Unhandled();
	if (RightClickDelegate.IsBound())
	{
		Reply = RightClickDelegate.Execute(MyGeometry, MouseEvent);
	}

	if (MouseEvent.GetModifierKeys().IsLeftShiftDown() && ShiftRightClickDelegate.IsBound())
	{
		ShiftRightClickDelegate.Execute();
	}

	FReply SuperReply = FReply::Unhandled();
	SuperReply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);

	// if its handled we should return it because it releases mouse capture!
	if (SuperReply.IsEventHandled())
	{
		return SuperReply;
	}

	return Reply;
}

//FReply SGroupButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
//{
//	FReply Reply = FReply::Unhandled();
//
//	
//
//	if (RightClickDelegate.IsBound())
//	{
//		Reply = RightClickDelegate.Execute(MyGeometry, MouseEvent);
//	}
//	else
//	{
//		Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
//	}
//
//	return Reply;
//}




END_SLATE_FUNCTION_BUILD_OPTIMIZATION
