// Copyright 2024 - NegativeNameSeller

#include "Factory/Factory_OpenLogicGraphCustomization.h"
#include "Classes/GraphCustomization.h"

UGraphCustomizationFactory::UGraphCustomizationFactory()
{
    SupportedClass = UGraphCustomization::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UGraphCustomizationFactory::ShouldShowInNewMenu() const
{
    return true;
}

UObject* UGraphCustomizationFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UGraphCustomization>(InParent, Class, Name, Flags, Context);
}