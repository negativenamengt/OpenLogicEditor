// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicNumber.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicDouble.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicDouble : public UOpenLogicNumber
{
	GENERATED_BODY()

public:
	UOpenLogicDouble();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
