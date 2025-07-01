// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_DoOnce.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicBoolean.h"

UTask_DoOnce::UTask_DoOnce(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Do Once";
	TaskData.Description = FText::FromString("Output fires only on the first time the node is hit, but can be reset");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);
	NodeLifecycle = ENodeLifecycle::Persistent;
	
	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("Reset"));
	TaskData.InputPins.Add(FOpenLogicPinData("Start Closed", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicBoolean::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Completed"));
}

void UTask_DoOnce::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	if (PinName == FName("Reset"))
	{
		bFirstEntrance = true;
		bIsClosed = false;
		return;	
	}

	bool bStartClosed = GetPropertyValueByAttribute<bool>(FName("Start Closed"));
	
	if (bFirstEntrance)
	{
		bFirstEntrance = false;

		if (bStartClosed)
		{
			bIsClosed = true;
		}
	}
	
	if (!bIsClosed)
	{
		bIsClosed = true;
		CompleteTask("Completed");
	}
}