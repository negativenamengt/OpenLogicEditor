// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicAny.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicAny : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicAny();
	virtual bool IsCompatibleWith_Implementation(UOpenLogicProperty* OtherProperty) override;
};