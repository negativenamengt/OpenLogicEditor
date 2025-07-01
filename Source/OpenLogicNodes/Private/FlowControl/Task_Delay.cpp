// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_Delay.h"

#include "DelayAction.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicFloat.h"
#include "Classes/Properties/OpenLogicObject.h"
#include "Classes/Properties/OpenLogicString.h"
#include "Classes/Properties/OpenLogicTransform.h"

UTask_Delay::UTask_Delay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Delay";
	TaskData.Description = FText::FromString("Perform a latent action with a delay (specified in seconds). Calling again while it is counting down will be ignored.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("Duration", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicFloat::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Completed"));
}

void UTask_Delay::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	float Duration = GetPropertyValueByAttribute<float>(FName("Duration"));

	if (Duration <= 0.0f)
	{
		OnDelayCompleted();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FLatentActionManager& LatentManager = World->GetLatentActionManager();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(UTask_Delay, OnDelayCompleted);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = GetUniqueID();

	if (!LatentManager.FindExistingAction<FDelayAction>(this, LatentInfo.UUID))
	{
		LatentManager.AddNewAction(this, LatentInfo.UUID, new FDelayAction(Duration, LatentInfo));
	}
}

void UTask_Delay::OnDelayCompleted()
{
	CompleteTask("Completed");
}
