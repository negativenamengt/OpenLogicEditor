// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_OpenLogicTaskCollection.generated.h"

UCLASS()
class UOpenLogicTaskCollectionFactory : public UFactory
{
    GENERATED_BODY()
public:
    UOpenLogicTaskCollectionFactory(const FObjectInitializer& ObjectInitializer);

public:
    virtual bool ShouldShowInNewMenu() const override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
};