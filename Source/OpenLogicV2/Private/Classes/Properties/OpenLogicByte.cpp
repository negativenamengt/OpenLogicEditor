// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicByte.h"

#include "Widgets/Input/SNumericEntryBox.h"

UOpenLogicByte::UOpenLogicByte()
{
	PropertyDisplayName = FText::FromString("Byte");
	PropertyColor = FLinearColor(0.003, 0.1, 0.08, 1.0);
	SupportedAttributes = ClampMin | ClampMax;
	UnderlyingType = EOpenLogicUnderlyingType::Byte;
}

TArray<TSharedRef<SWidget>> UOpenLogicByte::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return { CreateSpinBox<uint8>(DefaultValueHandle, PinAttributes) };
}