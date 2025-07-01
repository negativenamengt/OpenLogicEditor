// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicRuntimeGraph.h"

class OPENLOGICV2_API FOpenLogicGraphRunnable : public FRunnable
{
public:
	FOpenLogicGraphRunnable(UOpenLogicRuntimeGraph* InGraph)
		: Graph(InGraph)
	{
		QueueEvent = FPlatformProcess::GetSynchEventFromPool(true);
	}

	virtual ~FOpenLogicGraphRunnable()
	{
		if (QueueEvent)
		{
			// Clean up the event
			FPlatformProcess::ReturnSynchEventToPool(QueueEvent);
			QueueEvent = nullptr;
		}
	}

public:
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

public:
	void AddExecutionHandle(const FOpenLogicQueuedExecutionHandle& QueueData);
	bool IsRunning() const { return !bStopThread; }

private:
	void ProcessQueue();
	
private:
	FThreadSafeBool bStopThread;
	TQueue<FOpenLogicQueuedExecutionHandle, EQueueMode::Mpsc> Queue;
	FCriticalSection QueueLock;
	FEvent* QueueEvent;
	UOpenLogicRuntimeGraph* Graph;
};
