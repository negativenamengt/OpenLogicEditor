// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralInteger.h"	
#include "Classes/Properties/OpenLogicInteger.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralInteger::UTask_MakeLiteralInteger(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal Integer";
	TaskData.Description = FText::FromString("Creates a literal integer");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
}

void UTask_MakeLiteralInteger::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}