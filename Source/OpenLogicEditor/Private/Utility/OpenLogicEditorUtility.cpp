// Copyright 2024 - NegativeNameSeller

#include "Utility/OpenLogicEditorUtility.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Settings/OpenLogicEditorSettings.h"
#include "Blueprint/Blueprint_OpenLogicTask.h"
#include "Engine/ObjectLibrary.h"

void UOpenLogicEditorUtility::MarkTaskCollectionDirty(UOpenLogicTaskCollection* TaskCollection)
{
	if (!IsValid(TaskCollection))
	{
		return;
	}

	TaskCollection->Modify();
	TaskCollection->MarkPackageDirty();
}

TArray<FString> UOpenLogicEditorUtility::GetTaskAssetPaths()
{
	UOpenLogicEditorSettings* Settings = GetMutableDefault<UOpenLogicEditorSettings>();

	if (!Settings)
	{
		return TArray<FString>();
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter Filter;
	Filter.ClassPaths.Add(UOpenLogicTaskBlueprint::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Append(Settings->PathsToScan);
	Filter.bRecursiveClasses = true;
	Filter.bRecursivePaths = true;
	
	TArray<FAssetData> AssetDatas;
	AssetRegistry.GetAssets(Filter, AssetDatas);

	TArray<FString> Paths;
	
	for (FAssetData AssetData : AssetDatas)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
		{
			if (Blueprint && Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf(UOpenLogicTask::StaticClass()))
			{
				Paths.Add(AssetData.GetObjectPathString() + "_C");
			}
		}
	}

	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UOpenLogicTask::StaticClass(), DerivedClasses, true);

	for (UClass* DerivedClass : DerivedClasses)
	{
		if (DerivedClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}

		if (DerivedClass->ClassGeneratedBy != nullptr)
		{
			continue;
		}

		Paths.Add(DerivedClass->GetPathName());
	}
	
	return Paths;
}

TArray<TSoftClassPtr<UOpenLogicTask>> UOpenLogicEditorUtility::GetAllTaskSoftClasses()
{
	TArray<FString> Paths = UOpenLogicEditorUtility::GetTaskAssetPaths();
	TArray<TSoftClassPtr<UOpenLogicTask>> SoftClasses;

	for (FString Path : Paths)
	{
		TSoftClassPtr<UOpenLogicTask> SoftClass = TSoftClassPtr<UOpenLogicTask>(FSoftObjectPath(*Path));
		SoftClasses.Add(SoftClass);
	}

	return SoftClasses;
}