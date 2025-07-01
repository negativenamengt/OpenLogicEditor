// Copyright 2024 - NegativeNameSeller

#include "FlowControl/Task_DelayUntilNextTick.h"

#include "DelayAction.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicBoolean.h"

int LatentActionCVars::GuaranteeEngineTickDelay = 0;

UTask_DelayUntilNextTick::UTask_DelayUntilNextTick(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Delay Until Next Tick";
	TaskData.Description = FText::FromString("Perform a latent action with a delay of one tick. Calling again while it is counting down will be ignored.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("Completed"));
}

void UTask_DelayUntilNextTick::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	FLatentActionManager& LatentManager = World->GetLatentActionManager();
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(UTask_DelayUntilNextTick, OnDelayCompleted);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = GetUniqueID();

	if (!LatentManager.FindExistingAction<FDelayUntilNextTickAction>(this, LatentInfo.UUID))
	{
		LatentManager.AddNewAction(this, LatentInfo.UUID, new FDelayUntilNextTickAction(LatentInfo));
	}
}

void UTask_DelayUntilNextTick::OnDelayCompleted()
{
	CompleteTask("Completed");
}
