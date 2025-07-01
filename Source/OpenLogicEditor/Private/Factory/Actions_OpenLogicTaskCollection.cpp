// Copyright 2024 - NegativeNameSeller

#include "Factory/Actions_OpenLogicTaskCollection.h"
#include "Tasks/OpenLogicTaskCollection.h"
#include "OpenLogicEditor.h"

UClass* FAssetType_OpenLogicTaskCollection::GetSupportedClass() const
{
    return UOpenLogicTaskCollection::StaticClass();
}

FText FAssetType_OpenLogicTaskCollection::GetName() const
{
    return FText::FromString("Open Logic Task Collection");
}

FColor FAssetType_OpenLogicTaskCollection::GetTypeColor() const
{
    return FColor::Yellow;
}

uint32 FAssetType_OpenLogicTaskCollection::GetCategories()
{
    return FOpenLogicEditorModule::OpenLogic_AssetCategory;
}
