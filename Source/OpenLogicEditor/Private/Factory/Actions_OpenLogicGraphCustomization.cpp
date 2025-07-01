// Copyright 2024 - NegativeNameSeller

#include "Factory/Actions_OpenLogicGraphCustomization.h"
#include "Classes/GraphCustomization.h"
#include "OpenLogicEditor.h"

UClass* FAssetType_OpenLogicGraphCustomization::GetSupportedClass() const
{
    return UGraphCustomization::StaticClass();
}

FText FAssetType_OpenLogicGraphCustomization::GetName() const
{
    return FText::FromString("Open Logic Customization");
}

FColor FAssetType_OpenLogicGraphCustomization::GetTypeColor() const
{
    return FColor::Purple;
}

uint32 FAssetType_OpenLogicGraphCustomization::GetCategories()
{
    return FOpenLogicEditorModule::OpenLogic_AssetCategory;
}
