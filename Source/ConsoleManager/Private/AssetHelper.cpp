#include "AssetHelper.h"

#include "Engine/AssetManager.h"

AssetHelper::AssetHelper(TArray<FAssetData> Assets)
{

	//UAssetManager::Get().GetAssetRegistry().GetAssetsByClass(UCommandsContainer::StaticClass()->GetFName(), Assets);


	//for (auto& Asset : Assets)
	//{
	//	UPackage* Package = Asset.GetPackage();

	//	UObject* Resolved = Asset.GetAsset();

	//	if (Resolved)
	//	{
	//		UCommandsContainer* Container = Cast< UCommandsContainer >(Resolved);
	//		auto& Groups = CommandsManager.Get()->GetCommandGroups();

	//		for (int i = 0; i < Groups.Num(); i++)
	//		{
	//			Container->Groups.Add(Groups[i]);
	//		}
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Not resolved!"));
	//	}

	//	UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());
	//	// Construct a filename from long package name.
	//	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
	//	
	//	//FString Path = FString::Printf(TEXT("%s%s%s"));

	//	FSavePackageResultStruct OutStruct = Package->Save(Package, Asset.GetAsset(), EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *PackageFileName);
	//	//UPackage::Save(Package, Resolved, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, Asset.);
	//	//Resolved->PostEditChange();
	//	
	//	UE_LOG(LogTemp, Warning, TEXT("Result of save: %d | %lld"), OutStruct.Result, OutStruct.TotalFileSize);
	//	
	//}




	//for (auto& Asset : Assets)
	//{
	//	UPackage* Package = Asset.GetPackage();
	//	//Package->MarkPackageDirty();


	//	UObject* Resolved = Asset.GetAsset();

	//	if (Resolved)
	//	{
	//		UCommandsContainer* Container = Cast< UCommandsContainer >(Resolved);
	//		auto& Groups = CommandsManager.Get()->GetCommandGroups();

	//		for (int i = 0; i < Groups.Num(); i++)
	//		{
	//			Container->Groups.Add(Groups[i]);
	//		}
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("Not resolved!"));
	//	}

	//	UE_LOG(LogTemp, Warning, TEXT("Package name: %s | %s | %s"), *Package->FileName.ToString(), *Package->GetFullGroupName(false), *Package->GetPathName());
	//	// Construct a filename from long package name.
	//	FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetPathName(), FPackageName::GetAssetPackageExtension());
	//	
	//	//FString Path = FString::Printf(TEXT("%s%s%s"));

	//	FSavePackageResultStruct OutStruct = Package->Save(Package, Asset.GetAsset(), EObjectFlags::RF_Standalone | EObjectFlags::RF_Public, *PackageFileName);
	//	//UPackage::Save(Package, Resolved, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, Asset.);
	//	//Resolved->PostEditChange();
	//	
	//	UE_LOG(LogTemp, Warning, TEXT("Result of save: %d | %lld"), OutStruct.Result, OutStruct.TotalFileSize);
	//	
	//}



}
