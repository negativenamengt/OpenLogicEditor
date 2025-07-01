// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicProperty.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "OpenLogicVector.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicVector : public UOpenLogicProperty
{
	GENERATED_BODY()

public:
	UOpenLogicVector();

	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const override;
	static TSharedRef<SVectorInputBox> CreateVectorInputBox( UE::Math::TVector<float>& InitialValue, TFunction<void(const UE::Math::TVector<float>&, ETextCommit::Type)> OnVectorCommitted);
};