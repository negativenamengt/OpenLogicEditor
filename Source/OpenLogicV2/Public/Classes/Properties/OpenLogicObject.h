// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "OpenLogicObject.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicObject : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicObject();

#if WITH_EDITOR
	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
#endif
};
