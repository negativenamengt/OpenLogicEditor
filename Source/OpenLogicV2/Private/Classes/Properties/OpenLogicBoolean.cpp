// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicBoolean.h"

#include "Components/CheckBox.h"
#include "Widgets/ExecutionPinBase.h"

UOpenLogicBoolean::UOpenLogicBoolean()
{
	PropertyDisplayName = FText::FromString("Boolean");
	PropertyColor = FLinearColor(0.3, 0.0, 0.0, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::Boolean;
}

TArray<TSharedRef<SWidget>> UOpenLogicBoolean::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return { SNew(SCheckBox)
		.IsChecked_Lambda([DefaultValueHandle]()
		{
			bool bIsChecked = DefaultValueHandle->GetDefaultValue<bool>();
			return bIsChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda([DefaultValueHandle](ECheckBoxState NewState)
		{
			bool bIsChecked = NewState == ECheckBoxState::Checked;
			DefaultValueHandle->SetDefaultValue(bIsChecked);
		}) };
}