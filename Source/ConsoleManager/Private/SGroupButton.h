// Copyright 2021 Funky Fox Studio. All Rights Reserved.
// Author: Damian Baldyga

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SGroupButton : public SButton
{
public:
	/*SLATE_BEGIN_ARGS(SGroupButton)
	{}
	SLATE_END_ARGS()*/

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FPointerEventHandler RightClickDelegate;
	FSimpleDelegate ShiftRightClickDelegate;
	
protected:
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	//virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;


};
