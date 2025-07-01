// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_OpenLogicProperty.generated.h"

UCLASS()
class UOpenLogicPropertyFactory : public UFactory
{
    GENERATED_BODY()
public:
    UOpenLogicPropertyFactory(const FObjectInitializer& ObjectInitializer);

public:
    virtual bool ShouldShowInNewMenu() const override;
    virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

public:
    UPROPERTY()
        TSubclassOf<class UObject> ParentClass;
};