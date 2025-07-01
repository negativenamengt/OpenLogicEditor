// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralFloat.h"	
#include "Classes/Properties/OpenLogicFloat.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralFloat::UTask_MakeLiteralFloat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal Float";
	TaskData.Description = FText::FromString("Creates a literal float (single-precision)");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicFloat::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicFloat::StaticClass()));
}

void UTask_MakeLiteralFloat::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}