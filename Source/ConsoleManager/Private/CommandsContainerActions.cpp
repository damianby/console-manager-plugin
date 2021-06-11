#include "CommandsContainerActions.h"

#include "ContentBrowserModule.h"

#include "CommandsContainer.h"
#include "ConsoleManager.h"

#include "AssetRegistry/AssetData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"



/* FCommandsContainerActions constructors
 *****************************************************************************/

FCommandsContainerActions::FCommandsContainerActions()
{ }


/* FAssetTypeActions_Base overrides
 *****************************************************************************/

FText FCommandsContainerActions::GetAssetDescription(const FAssetData& AssetData) const
{
	FText Desc;
	UObject* Obj = AssetData.FastGetAsset();
	if (Obj)
	{
		UCommandsContainer* Container = Cast< UCommandsContainer >(Obj);
		
		int32 TotalCommands = 0;
		for (const auto& Group : Container->Groups)
		{
			TotalCommands += Group.Commands.Num();
		}

		FString Text = FString::Printf(TEXT("Contains %d groups with %d commands"), Container->Groups.Num(), TotalCommands);

		Desc = FText::FromString(Text);
	}

	return Desc;
}

bool FCommandsContainerActions::CanFilter()
{
	return true;
}


void FCommandsContainerActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}


uint32 FCommandsContainerActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}


FText FCommandsContainerActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CommandsContainer", "Commands Container");
}


UClass* FCommandsContainerActions::GetSupportedClass() const
{
	return UCommandsContainer::StaticClass();
}


FColor FCommandsContainerActions::GetTypeColor() const
{
	return FColor::White;
}


bool FCommandsContainerActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}


void FCommandsContainerActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{

	TArray<UCommandsContainer*> OutCommandsContainers;

	for (const auto& Container : InObjects)
	{
		UCommandsContainer* OutContainer = StaticCast<UCommandsContainer*>(Container);
		OutCommandsContainers.Add(OutContainer);
	}

	FConsoleManagerModule::GetModule().OpenTabs(OutCommandsContainers);

	//EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
	//	? EToolkitMode::WorldCentric
	//	: EToolkitMode::Standalone;

	//for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	//{
	//	auto Container = Cast<UCommandsContainer>(*ObjIt);

	//	if (Container != nullptr)
	//	{
	//		//TSharedRef<FTextAssetEditorToolkit> EditorToolkit = MakeShareable(new FTextAssetEditorToolkit(Style));
	//		//EditorToolkit->Initialize(TextAsset, Mode, EditWithinLevelEditor);
	//	}
	//}
}


#undef LOCTEXT_NAMESPACE