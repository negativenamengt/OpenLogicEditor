// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_ForLoopWithBreak.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicInteger.h"

UTask_ForLoopWithBreak::UTask_ForLoopWithBreak(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "For Loop with Break";
	TaskData.Description = FText::FromString("Executes LoopBody for each index FirstIndex to LastIndex, inclusive.  If execution goes in to break, the loop will exit without executing the remaining indices");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("First Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.InputPins.Add(FOpenLogicPinData("Last Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.InputPins.Add(FOpenLogicPinData("Break"));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Loop Body"));
	TaskData.OutputPins.Add(FOpenLogicPinData("Index", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.OutputPins.Add(FOpenLogicPinData("Completed"));
}

void UTask_ForLoopWithBreak::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	if (PinName == "Break")
	{
		bBreak = true;
		return;
	}

	bBreak = false;
	
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

		if (bBreak)
		{
			break;
		}
	}

	CompleteTask("Completed");
}