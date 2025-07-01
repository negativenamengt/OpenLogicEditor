// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralBoolean.h"
#include "Classes/Properties/OpenLogicBoolean.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralBoolean::UTask_MakeLiteralBoolean(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal Boolean";
	TaskData.Description = FText::FromString("Creates a literal boolean");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicBoolean::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicBoolean::StaticClass()));
}

void UTask_MakeLiteralBoolean::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}
