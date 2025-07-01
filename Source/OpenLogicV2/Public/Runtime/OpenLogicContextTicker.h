// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

class UOpenLogicTask;

class OPENLOGICV2_API FOpenLogicContextTicker : public FTickableGameObject
{
private:
	TSet<UOpenLogicTask*> RegisteredTasks;

public:
	void RegisterTask(UOpenLogicTask* TaskToRegister)
	{
		RegisteredTasks.Add(TaskToRegister);
	}

	void UnregisterTask(UOpenLogicTask* TaskToUnregister)
	{
		RegisteredTasks.Remove(TaskToUnregister);
	}

	TSet<UOpenLogicTask*> GetActiveTasks()
	{
		return RegisteredTasks;
	}

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
	}

	virtual bool IsTickableInEditor() const
	{
		return false;
	}

	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}

private:
	uint32 LastFrameNumber = -1;
};
