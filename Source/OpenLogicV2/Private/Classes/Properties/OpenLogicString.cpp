// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicString.h"

UOpenLogicString::UOpenLogicString()
{
	PropertyDisplayName = FText::FromString("String");
	PropertyColor = FLinearColor(0.65, 0.0, 0.43, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::String;
}

TArray<TSharedRef<SWidget>> UOpenLogicString::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {
		SNew(SEditableTextBox)
			.Text_Lambda([DefaultValueHandle]()
			{
				FString Value = DefaultValueHandle->GetDefaultValue<FString>();
				return FText::FromString(Value);
			})
			.OnTextCommitted_Lambda([DefaultValueHandle](const FText& NewValue, ETextCommit::Type CommitType) mutable
			{
				DefaultValueHandle->SetDefaultValue(NewValue.ToString());
			})
	};
}
