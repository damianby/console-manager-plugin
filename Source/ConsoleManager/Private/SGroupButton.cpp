// Fill out your copyright notice in the Description page of Project Settings.


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
	if (RightClickDelegate.IsBound())
	{
		RightClickDelegate.Execute(MyGeometry, MouseEvent);
	}

	return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
}




END_SLATE_FUNCTION_BUILD_OPTIMIZATION
