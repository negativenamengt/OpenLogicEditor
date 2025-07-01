// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicNumber.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicByte.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicByte : public UOpenLogicNumber
{
	GENERATED_BODY()

public:
	UOpenLogicByte();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
