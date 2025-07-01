// Copyright 2025 - NegativeNameSeller

#include "Literal/Task_MakeLiteralDouble.h"	
#include "Classes/Properties/OpenLogicDouble.h"
#include "NodeLibraryTags.h"

UTask_MakeLiteralDouble::UTask_MakeLiteralDouble(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make Literal Double";
	TaskData.Description = FText::FromString("Creates a literal double (double-precision)");
	TaskData.Category = "Literal";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicLiteralLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicDouble::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Return Value", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicDouble::StaticClass()));
}

void UTask_MakeLiteralDouble::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	Super::OnTaskActivated_Implementation(Context, PinName);
}