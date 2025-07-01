// Copyright 2024 - NegativeNameSeller

#include "Factory/Factory_OpenLogicTaskCollection.h"
#include "Tasks/OpenLogicTaskCollection.h"
#include "Kismet2/KismetEditorUtilities.h"

UOpenLogicTaskCollectionFactory::UOpenLogicTaskCollectionFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = UOpenLogicTaskCollection::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UOpenLogicTaskCollectionFactory::ShouldShowInNewMenu() const
{
    return true;
}

UObject* UOpenLogicTaskCollectionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
    return NewObject<UOpenLogicTaskCollection>(InParent, Class, Name, Flags);
}