// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_ForLoop.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicInteger.h"

UTask_ForLoop::UTask_ForLoop(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "For Loop";
	TaskData.Description = FText::FromString("Executes LoopBody for each Index from StartIndex to EndIndex, inclusive.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("First Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.InputPins.Add(FOpenLogicPinData("Last Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Loop Body"));
	TaskData.OutputPins.Add(FOpenLogicPinData("Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.OutputPins.Add(FOpenLogicPinData("Completed"));
}

void UTask_ForLoop::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	int32 FirstIndex = GetPropertyValueByAttribute<int32>(FName("First Index"));
	int32 LastIndex = GetPropertyValueByAttribute<int32>(FName("Last Index"));

	if (FirstIndex > LastIndex)
	{
		CompleteTask("Completed");
		return;
	}

	for (int32 Index = FirstIndex; Index <= LastIndex; Index++)
	{
		SetPropertyValueByAttribute<int32>(FName("Index"), Index);
		ExecutePinByName("Loop Body");
	}

	CompleteTask("Completed");
}