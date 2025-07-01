// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralByte.h"
#include "Classes/Properties/OpenLogicByte.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralByte::UTask_MakeLiteralByte(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal Byte";
	TaskData.Description = FText::FromString("Creates a literal byte");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicByte::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicByte::StaticClass()));
}

void UTask_MakeLiteralByte::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}