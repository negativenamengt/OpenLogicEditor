// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicRotator.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicRotator : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicRotator();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
	static TSharedRef<SRotatorInputBox> CreateRotatorInputBox(FRotator InitialValue, TFunction<void(const FRotator&, ETextCommit::Type)> OnRotatorCommitted);
};
