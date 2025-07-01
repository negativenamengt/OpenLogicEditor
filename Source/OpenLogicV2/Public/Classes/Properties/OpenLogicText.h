// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicText.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicText : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicText();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
