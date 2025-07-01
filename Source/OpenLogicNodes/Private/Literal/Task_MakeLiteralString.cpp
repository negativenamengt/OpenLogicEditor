// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralString.h"	
#include "Classes/Properties/OpenLogicString.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralString::UTask_MakeLiteralString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal String";
	TaskData.Description = FText::FromString("Creates a literal string");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicString::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicString::StaticClass()));
}

void UTask_MakeLiteralString::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}