// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_OpenLogicTask.generated.h"

UCLASS()
class UOpenLogicTaskFactory : public UFactory
{
    GENERATED_BODY()
public:
    UOpenLogicTaskFactory(const FObjectInitializer& ObjectInitializer);

public:
    virtual bool ShouldShowInNewMenu() const override;
    virtual bool ConfigureProperties() override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

public:
    UPROPERTY()
        TSubclassOf<class UObject> ParentClass;
};