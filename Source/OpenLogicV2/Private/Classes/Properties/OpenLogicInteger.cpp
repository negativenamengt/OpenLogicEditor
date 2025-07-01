// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicInteger.h"

#include "Core/OpenLogicTypes.h"
#include "Widgets/Input/SNumericEntryBox.h"

UOpenLogicInteger::UOpenLogicInteger()
{
	PropertyDisplayName = FText::FromString("Integer");
	PropertyColor = FLinearColor(0.009, 0.33, 0.19, 1.0);
	SupportedAttributes = ClampMin | ClampMax;
	UnderlyingType = EOpenLogicUnderlyingType::Int;
}

TArray<TSharedRef<SWidget>> UOpenLogicInteger::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {
		CreateSpinBox<int32>(DefaultValueHandle, PinAttributes)
	};
}
