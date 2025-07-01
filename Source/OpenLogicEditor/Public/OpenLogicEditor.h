// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Factory/Actions_OpenLogicGraph.h"
#include "Factory/Actions_OpenLogicGraphCustomization.h"
#include "Modules/ModuleManager.h"
#include "Factory/Actions_OpenLogicTask.h"
#include "Factory/Actions_OpenLogicTaskCollection.h"
#include "Factory/Actions_OpenLogicProperty.h"

class FOpenLogicEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	static EAssetTypeCategories::Type OpenLogic_AssetCategory;

private:
	// Called when the asset registry has loaded all the files.
	void OnAssetRegistryFilesLoaded();

	FSoftObjectPath TaskGeneratorToolPath = FSoftObjectPath("/OpenLogicV2/OpenLogicTools/EUW_OpenLogicTasksGenerator.EUW_OpenLogicTasksGenerator");

private:
	// Called when a widget class has been reloaded.
	void OnReloadComplete(UObject* Object, const FObjectPostCDOCompiledContext& Context);

private:
	TArray<TSharedPtr<FAssetTypeActions_Base>> OpenLogic_AssetActions = {
		TSharedPtr<FAssetTypeActions_Base>(new FAssetType_OpenLogicTask),
		TSharedPtr<FAssetTypeActions_Base>(new FAssetType_OpenLogicTaskCollection),
		TSharedPtr<FAssetTypeActions_Base>(new FAssetType_OpenLogicProperty),
		TSharedPtr<FAssetTypeActions_Base>(new FAssetType_OpenLogicGraph),
		TSharedPtr<FAssetTypeActions_Base>(new FAssetType_OpenLogicGraphCustomization)
	};

};

DECLARE_LOG_CATEGORY_EXTERN(OpenLogicEditorLog, Log, All);