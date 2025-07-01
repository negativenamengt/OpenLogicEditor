// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicDouble.h"

#include "Widgets/Input/SNumericEntryBox.h"

UOpenLogicDouble::UOpenLogicDouble()
{
	PropertyDisplayName = FText::FromString("Double");
	PropertyColor = FLinearColor(0.03, 0.66, 0.0, 1.0);
	SupportedAttributes = ClampMin | ClampMax;
	UnderlyingType = EOpenLogicUnderlyingType::Double;
}

TArray<TSharedRef<SWidget>> UOpenLogicDouble::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {
		CreateSpinBox<double>(DefaultValueHandle, PinAttributes)
	};
}