// Fill out your copyright notice in the Description page of Project Settings.

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
	
protected:
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;


};
