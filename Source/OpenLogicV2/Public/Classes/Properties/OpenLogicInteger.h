// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicNumber.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicInteger.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicInteger : public UOpenLogicNumber
{
	GENERATED_BODY()

public:
	UOpenLogicInteger();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
