// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicString.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicString : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicString();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
};
