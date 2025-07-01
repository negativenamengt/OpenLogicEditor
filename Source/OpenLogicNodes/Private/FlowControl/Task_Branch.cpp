// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_Branch.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicBoolean.h"

UTask_Branch::UTask_Branch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Branch (If)";
	TaskData.Description = FText::FromString("If Condition is true, 'True' will be executed, otherwise 'False'.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("Condition", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicBoolean::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("True"));
	TaskData.OutputPins.Add(FOpenLogicPinData("False"));
}

void UTask_Branch::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	if (GetPropertyValueByAttribute<bool>(FName("Condition")))
	{
		CompleteTask("True");
	} else
	{
		CompleteTask("False");
	}
}
