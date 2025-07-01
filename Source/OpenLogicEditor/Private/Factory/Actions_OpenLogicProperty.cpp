// Copyright 2024 - NegativeNameSeller

#include "Factory/Actions_OpenLogicProperty.h"
#include "OpenLogicEditor.h"
#include "Blueprint/Blueprint_OpenLogicProperty.h"
#include "Factory/Factory_OpenLogicProperty.h"

UClass* FAssetType_OpenLogicProperty::GetSupportedClass() const
{
    return UOpenLogicPropertyBlueprint::StaticClass();
}

FText FAssetType_OpenLogicProperty::GetName() const
{
    return FText::FromString("Open Logic Property");
}

FColor FAssetType_OpenLogicProperty::GetTypeColor() const
{
    return FColor::Silver;
}

uint32 FAssetType_OpenLogicProperty::GetCategories()
{
    return FOpenLogicEditorModule::OpenLogic_AssetCategory;
}

UFactory* FAssetType_OpenLogicProperty::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
    UOpenLogicPropertyFactory* BlueprintFactory = NewObject<UOpenLogicPropertyFactory>();
    BlueprintFactory->ParentClass = InBlueprint->GeneratedClass;
    return BlueprintFactory;
}
