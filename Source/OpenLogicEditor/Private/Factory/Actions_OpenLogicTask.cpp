// Copyright 2024 - NegativeNameSeller

#include "Factory/Actions_OpenLogicTask.h"
#include "Blueprint/Blueprint_OpenLogicTask.h"
#include "Factory/Factory_OpenLogicTask.h"
#include "OpenLogicEditor.h"

UClass* FAssetType_OpenLogicTask::GetSupportedClass() const
{
	return UOpenLogicTaskBlueprint::StaticClass();
}

FText FAssetType_OpenLogicTask::GetName() const
{
	return FText::FromString("Open Logic Task");
}

FColor FAssetType_OpenLogicTask::GetTypeColor() const
{
	return FColor::FromHex("#3f7eff");
}

uint32 FAssetType_OpenLogicTask::GetCategories()
{
	return FOpenLogicEditorModule::OpenLogic_AssetCategory;
}

UFactory* FAssetType_OpenLogicTask::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UOpenLogicTaskFactory* BlueprintFactory = NewObject<UOpenLogicTaskFactory>();
	BlueprintFactory->ParentClass = InBlueprint->GeneratedClass;
	return BlueprintFactory;
}
