// Copyright 2024 - NegativeNameSeller

#include "Event/Task_OnGraphEnd.h"
#include "NodeLibraryTags.h"

UTask_OnGraphEnd::UTask_OnGraphEnd(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "On Graph End";
	TaskData.Type = ENodeType::Event;
	TaskData.Category = "Add Event";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicEventLibrary);

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("then"));
}

void UTask_OnGraphEnd::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	CompleteTask("then");
}
