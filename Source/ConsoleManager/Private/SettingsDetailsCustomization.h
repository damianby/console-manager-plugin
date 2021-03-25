// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "ConsoleManagerSettings.h"

/**
 * 
 */
class FSettingsDetailsCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;


	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FSettingsDetailsCustomization);
	}

private:

	TWeakObjectPtr<class UConsoleManagerSettings> Settings;


};
