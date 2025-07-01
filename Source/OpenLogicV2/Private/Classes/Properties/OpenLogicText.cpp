#include "Classes/Properties/OpenLogicText.h"

#if WITH_EDITOR
#endif

UOpenLogicText::UOpenLogicText()
{
	PropertyDisplayName = FText::FromString("Text");
	PropertyColor = FLinearColor(0.77, 0.19, 0.38, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::Text;
}

TArray<TSharedRef<SWidget>> UOpenLogicText::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {};
}