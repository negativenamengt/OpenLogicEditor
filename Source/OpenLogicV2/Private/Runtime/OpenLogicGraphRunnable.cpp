// Copyright 2024 - NegativeNameSeller

#include "Runtime/OpenLogicGraphRunnable.h"
#include "Runtime/OpenLogicRuntimeEventContext.h"

bool FOpenLogicGraphRunnable::Init()
{
	return Graph != nullptr;
}

uint32 FOpenLogicGraphRunnable::Run()
{
	while (IsRunning())
	{
		QueueEvent->Wait();

		if (!IsRunning())
			break;
		
		ProcessQueue();		
	}

	return 0;
}

void FOpenLogicGraphRunnable::Stop()
{
	bStopThread = true;
	
	// Wake up the thread to ensure it stops
	QueueEvent->Trigger();
}

void FOpenLogicGraphRunnable::AddExecutionHandle(const FOpenLogicQueuedExecutionHandle& QueueData)
{
	FScopeLock Lock(&QueueLock);
	Queue.Enqueue(QueueData);

	QueueEvent->Trigger();
}

void FOpenLogicGraphRunnable::ProcessQueue()
{
	FOpenLogicQueuedExecutionHandle QueuedNode;
	bool bHasProcessed = false;

	while (Queue.Dequeue(QueuedNode))
	{
		bHasProcessed = true;

		TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle= Graph->GetExecutionHandle(QueuedNode.HandleIndex);
		if (!ExecutionHandle.IsValid())
		{
			continue;
		}

		TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = Graph->ProcessNodeByGUID(ExecutionHandle->NodeID, ExecutionHandle);
		if (!RuntimeNode.IsValid())
		{
			continue;
		}

		Graph->ActivateNode(RuntimeNode, NAME_None);
	}

	if (bHasProcessed)
	{
		FScopeLock Lock(&QueueLock);
		if (Queue.IsEmpty())
		{
			QueueEvent->Reset();
		}
	}
}
