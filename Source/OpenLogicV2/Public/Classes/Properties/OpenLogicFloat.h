// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicNumber.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicFloat.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicFloat : public UOpenLogicNumber
{
	GENERATED_BODY()

public:
	UOpenLogicFloat();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
