// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicBoolean.generated.h"

class UExecutionPinBase;

UCLASS()
class OPENLOGICV2_API UOpenLogicBoolean : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicBoolean();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};