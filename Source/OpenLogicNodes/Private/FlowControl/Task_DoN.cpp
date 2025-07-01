// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_DoN.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicInteger.h"

UTask_DoN::UTask_DoN(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Do N";
	TaskData.Description = FText::FromString("Allows exec to pass through only 'n' times.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);
	NodeLifecycle = ENodeLifecycle::Persistent;

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("N", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
	TaskData.InputPins.Add(FOpenLogicPinData("Reset"));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Exit"));
	TaskData.OutputPins.Add(FOpenLogicPinData("Counter", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicInteger::StaticClass()));
}

void UTask_DoN::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	if (PinName == "Reset")
	{
		Counter = 0;
		return;
	}
	
	int32 N = GetPropertyValueByAttribute<int32>(FName("N"));

	if (Counter < N)
	{
		Counter++;
		SetPropertyValueByAttribute<int32>(FName("Counter"), Counter);
		ExecutePinByName("execute");
	}
}