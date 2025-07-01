// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicFloat.h"

UOpenLogicFloat::UOpenLogicFloat()
{
	PropertyDisplayName = FText::FromString("Float");
	PropertyColor = FLinearColor(0.03, 0.63, 0.0, 1.0);
	SupportedAttributes = ClampMin | ClampMax;
	UnderlyingType = EOpenLogicUnderlyingType::Float;
}

TArray<TSharedRef<SWidget>> UOpenLogicFloat::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {
		CreateSpinBox<float>(DefaultValueHandle, PinAttributes)
	};
}