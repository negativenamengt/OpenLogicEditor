// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicVector.h"

#include "Components/TextBlock.h"
#include "Widgets/Input/SVectorInputBox.h"

UOpenLogicVector::UOpenLogicVector()
{
	PropertyDisplayName = FText::FromString("Vector");
	PropertyColor = FLinearColor(0.87, 0.52, 0.0, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::Struct;
	StructType = TBaseStructure<FVector>::Get();
}

TArray<TSharedRef<SWidget>> UOpenLogicVector::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	FVector InitialValue = DefaultValueHandle->GetDefaultValue<FVector>();
	UE::Math::TVector<float> InitialVector(InitialValue.X, InitialValue.Y, InitialValue.Z);
	
	return {
		CreateVectorInputBox(
		InitialVector,
			[DefaultValueHandle](const UE::Math::TVector<float>& NewValue, ETextCommit::Type CommitType)
			{
				FVector NewVector(NewValue.X, NewValue.Y, NewValue.Z);
				DefaultValueHandle->SetDefaultValue(NewVector);
			}
		)
	};
}

TSharedRef<SVectorInputBox> UOpenLogicVector::CreateVectorInputBox(UE::Math::TVector<float>& InitialValue, TFunction<void(const UE::Math::TVector<float>&, ETextCommit::Type)> OnVectorCommitted)
{
    TSharedPtr<UE::Math::TVector<float>> CurrentValue = MakeShared<UE::Math::TVector<float>>(InitialValue);

    return SNew(SNumericVectorInputBox<float>)
    	.AllowSpin(true)
    	.bColorAxisLabels(true)
        .Vector_Lambda([CurrentValue]() -> UE::Math::TVector<float>
		{
			return *CurrentValue;
		})
        .OnXChanged_Lambda([CurrentValue](float NewX)
        {
            CurrentValue->X = NewX;
        })
        .OnYChanged_Lambda([CurrentValue](float NewY)
        {
            CurrentValue->Y = NewY;
        })
        .OnZChanged_Lambda([CurrentValue](float NewZ)
        {
            CurrentValue->Z = NewZ;
        })

        // Per-component committed handlers
        .OnXCommitted_Lambda([CurrentValue, OnVectorCommitted](float NewX, ETextCommit::Type CommitType)
        {
            CurrentValue->X = NewX;

            if (OnVectorCommitted)
            {
                OnVectorCommitted(*CurrentValue, CommitType);    
            }
        })
        .OnYCommitted_Lambda([CurrentValue, OnVectorCommitted](float NewY, ETextCommit::Type CommitType)
        {
            CurrentValue->Y = NewY;

            if (OnVectorCommitted)
            {
                OnVectorCommitted(*CurrentValue, CommitType);    
            }
        })
        .OnZCommitted_Lambda([CurrentValue, OnVectorCommitted](float NewZ, ETextCommit::Type CommitType)
        {
            CurrentValue->Z = NewZ;

            if (OnVectorCommitted)
            {
                OnVectorCommitted(*CurrentValue, CommitType);    
            }
        });
}