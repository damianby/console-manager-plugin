// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsDetailsCustomization.h"

#include "DetailCategoryBuilder.h"

void FSettingsDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

	TArray<TWeakObjectPtr<UObject>> SelectedObjects; 

	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);

	TArray<UConsoleManagerSettings*> SettingsObjects;

	for (auto& Object : SelectedObjects)
	{
		if (Object.IsValid())
		{
			UConsoleManagerSettings* SettingsObj = Cast<UConsoleManagerSettings>(Object.Get());
			
			SettingsObjects.Add(SettingsObj);
		}
	}

	IDetailCategoryBuilder& ShortcutsCategory = DetailBuilder.EditCategory("Shortcuts");

	TSharedPtr<IPropertyHandle> Shortcut = DetailBuilder.GetProperty("OpenShortcut");

	/*Shortcut->CreatePropertyValueWidget(false)
		[
			SNew(STextBlock)
			.Text(FText::FromString("ELO"))
		];*/

	if (Shortcut->IsValidHandle())
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle is valid!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle IS NOT VALID!!!"));
	}



}
