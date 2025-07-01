// Copyright 2024 - NegativeNameSeller

#include "Event/Task_OnGraphStart.h"
#include "NodeLibraryTags.h"

UTask_OnGraphStart::UTask_OnGraphStart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "On Graph Start";
	TaskData.Type = ENodeType::Event;
	TaskData.Category = "Add Event";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicEventLibrary);

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("then"));
}

void UTask_OnGraphStart::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	CompleteTask("then");
}
