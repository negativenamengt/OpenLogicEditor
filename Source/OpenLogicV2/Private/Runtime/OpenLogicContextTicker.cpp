// Copyright 2024 - NegativeNameSeller

#include "Runtime/OpenLogicContextTicker.h"
#include "Tasks/OpenLogicTask.h"

void FOpenLogicContextTicker::Tick(float DeltaTime)
{
	if (LastFrameNumber == GFrameCounter)
	{
		return;
	}

	// Creates a temporary copy of the registered tasks
	TSet<UOpenLogicTask*> TempRegisteredTasks = RegisteredTasks;

	for (UOpenLogicTask* Task : TempRegisteredTasks)
	{
		Task->OnTaskTick(DeltaTime);
	}

	// Update LastFrameNumber
	LastFrameNumber = GFrameCounter;
}