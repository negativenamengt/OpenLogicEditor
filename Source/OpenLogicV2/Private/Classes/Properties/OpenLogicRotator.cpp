// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicRotator.h"

#include "Widgets/Input/SRotatorInputBox.h"

UOpenLogicRotator::UOpenLogicRotator()
{
	PropertyDisplayName = FText::FromString("Rotator");
	PropertyColor = FLinearColor(0.19, 0.24, 0.51, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::Struct;
	StructType = TBaseStructure<FRotator>::Get();
}

TArray<TSharedRef<SWidget>> UOpenLogicRotator::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	return {
		CreateRotatorInputBox(
			DefaultValueHandle->GetDefaultValue<FRotator>(),
			[DefaultValueHandle](const FRotator& NewValue, ETextCommit::Type CommitType)
			{
				DefaultValueHandle->SetDefaultValue(NewValue);
			}
		)
	};
}

TSharedRef<SRotatorInputBox> UOpenLogicRotator::CreateRotatorInputBox(FRotator InitialValue, TFunction<void(const FRotator&, ETextCommit::Type)> OnRotatorCommitted)
{
	TSharedPtr<FRotator> CurrentValue = MakeShared<FRotator>(InitialValue);
	
	return SNew(SRotatorInputBox)
		.AllowSpin(true)
		.bColorAxisLabels(true)
		.Roll_Lambda([CurrentValue]() -> float
		{
			return CurrentValue->Roll;
		})
		.Pitch_Lambda([CurrentValue]() -> float
		{
			return CurrentValue->Pitch;
		})
		.Yaw_Lambda([CurrentValue]() -> float
		{
			return CurrentValue->Yaw;
		})
		.OnYawChanged_Lambda([CurrentValue](float NewYaw)
		{
			CurrentValue->Yaw = NewYaw;
		})
		.OnPitchChanged_Lambda([CurrentValue](float NewPitch)
		{
			CurrentValue->Pitch = NewPitch;
		})
		.OnRollChanged_Lambda([CurrentValue](float NewRoll)
		{
			CurrentValue->Roll = NewRoll;
		})
		.OnYawCommitted_Lambda([CurrentValue, OnRotatorCommitted](float NewYaw, ETextCommit::Type CommitType)
		{
			CurrentValue->Yaw = NewYaw;

			if (OnRotatorCommitted)
			{
				OnRotatorCommitted(*CurrentValue, CommitType);
			}
		})
		.OnPitchCommitted_Lambda([CurrentValue, OnRotatorCommitted](float NewPitch, ETextCommit::Type CommitType)
		{
			CurrentValue->Pitch = NewPitch;

			if (OnRotatorCommitted)
			{
				OnRotatorCommitted(*CurrentValue, CommitType);
			}
		})
		.OnRollCommitted_Lambda([CurrentValue, OnRotatorCommitted](float NewRoll, ETextCommit::Type CommitType)
		{
			CurrentValue->Roll = NewRoll;

			if (OnRotatorCommitted)
			{
				OnRotatorCommitted(*CurrentValue, CommitType);
			}
		});
}
