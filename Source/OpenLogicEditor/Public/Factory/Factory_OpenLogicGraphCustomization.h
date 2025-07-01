// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "Factory_OpenLogicGraphCustomization.generated.h"

UCLASS()
class OPENLOGICEDITOR_API UGraphCustomizationFactory : public UFactory
{
	GENERATED_BODY()
public:
	UGraphCustomizationFactory();

public:
	virtual bool ShouldShowInNewMenu() const override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
