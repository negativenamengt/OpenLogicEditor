// Copyright 2024 - NegativeNameSeller

#include "Runtime/OpenLogicRuntimeEventContext.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "OpenLogicV2.h"
#include "Async/Async.h"
#include "Utility/PayloadObject.h"

void UOpenLogicRuntimeEventContext::ProcessEventNode()
{
	if (!GetWorker() || bProcessed || !EventID.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ProcessEvent] Couldn't process event."));
		return;
	}

	bProcessed = true;

	ProcessNodeByGuid(EventID);
	bRunning = true;
}

bool UOpenLogicRuntimeEventContext::IsProcessed() const
{
	return bProcessed;
}

bool UOpenLogicRuntimeEventContext::IsRunning() const
{
	return bRunning;
}

UOpenLogicTask* UOpenLogicRuntimeEventContext::TryGetInitialEvent()
{
	if (RuntimeNodes.IsEmpty() || !RuntimeNodes.Contains(EventID))
    {
        return nullptr;
    }

	return RuntimeNodes[EventID].TaskInstance;
}

UOpenLogicRuntimeGraph* UOpenLogicRuntimeEventContext::GetWorker() const
{
	return Worker;
}

void UOpenLogicRuntimeEventContext::Shutdown()
{
	for (const TPair<FGuid, FOpenLogicRuntimeNode>& RuntimeNode : RuntimeNodes)
	{
		if (RuntimeNode.Value.TaskInstance)
		{
			GetWorld()->GetLatentActionManager().RemoveActionsForObject(RuntimeNode.Value.TaskInstance);
			RuntimeNode.Value.TaskInstance->ConditionalBeginDestroy();
			RuntimeNode.Value.TaskInstance->MarkAsGarbage();
		}
	}
}

void UOpenLogicRuntimeEventContext::ProcessNodeByGuid(FGuid NodeGuid)
{
	if (!NodeGuid.IsValid())
	{
		return;
	}

	FOpenLogicRuntimeNode RuntimeNode = GetOrCreateRuntimeNode(NodeGuid);
	if (!RuntimeNode.TaskInstance)
	{
		return;
	}

	// Initializing the runtime node
	FOpenLogicGraphData GraphData = GetWorker()->GetGraphData();
	FOpenLogicNode NodeData = GetGraphNodeData(NodeGuid);
	FTaskData TaskData = RuntimeNode.TaskInstance->TaskData;
	UOpenLogicTask* TaskInstance = RuntimeNode.TaskInstance;
	
	if (RuntimeNode.TaskInstance->GetDynamicProperties().IsEmpty())
	{
		for (const TPair<int32, FOpenLogicPinState>& PinState : NodeData.OutputPins)
		{
			FOpenLogicPinData PinData = PinState.Value.PinData;

			if (!PinState.Value.IsUserCreated)
			{
				PinData = TaskData.OutputPins[PinState.Key - 1];
			}

			if (UClass* PropertyClass = PinData.PropertyClass.Get())
			{
				UOpenLogicProperty* NewProperty = NewObject<UOpenLogicProperty>(TaskInstance, PropertyClass);
				TaskInstance->GetDynamicProperties().Add(PinState.Key, NewProperty);
			}
		}
	}

	for (const TPair<FGuid, FName> BlueprintContent : NodeData.BlueprintContent)
	{
		FProperty* Property = FindFProperty<FProperty>(TaskInstance->GetClass(), TaskInstance->GetPropertyNameByGUID(BlueprintContent.Key));
		if (!Property)
		{
			continue;
		}

		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(TaskInstance);

		bool bSuccess;
		UPayloadObject* PayloadObject = UPayloadObject::Parse(BlueprintContent.Value.ToString(), bSuccess);
		if (bSuccess)
		{
			PayloadObject->CreateWildcardValue(PayloadObject, Property, ValuePtr);
		}
	}

	RuntimeNode.TaskState = EOpenLogicTaskState::Initialized;
	RuntimeNode.InputPinsCount = NodeData.InputPins.Num();
	RuntimeNode.OutputPinsCount = NodeData.OutputPins.Num();

	// Set the runtime node back to the map
	RuntimeNodes[NodeGuid] = RuntimeNode;
	
	// Alert the runtime graph that the node has been initialized
	GetWorker()->OnNodeActivated.Broadcast(TaskInstance);
	
	// Execute the node
	if (GetWorker()->GetThreadSettings().NodeExecutionThread == EOpenLogicRuntimeThreadType::GameThread)
	{
		// Update the task state
		RuntimeNode.TaskState = EOpenLogicTaskState::Running;

		// Set the runtime node back to the map
		RuntimeNodes[NodeGuid] = RuntimeNode;
		
		// Execute the node on the game thread
		TaskInstance->OnTaskActivated(GetWorker()->GetOuter());
	} else
	{

	}
}

bool UOpenLogicRuntimeEventContext::Then(FOpenLogicRuntimeNode& SourceNode, int32 OutputPinIndex)
{
	if (!SourceNode.TaskInstance || !SourceNode.TaskInstance->IsValidLowLevel())
    {
        return false;
    }

	if (OutputPinIndex < 1 || OutputPinIndex > SourceNode.OutputPinsCount)
    {
        return false;
    }

	FOpenLogicNode NodeData = GetGraphNodeData(SourceNode.TaskInstance->GetGuid());
	if (!NodeData.OutputPins.Contains(OutputPinIndex))
    {
        return false;
    }

	FOpenLogicPinState PinState = NodeData.OutputPins[OutputPinIndex];
	if (PinState.Connections.Num() == 0)
    {
        return false;
    }

	FGuid NextNodeGuid = PinState.Connections[0].NodeID;
	if (!NextNodeGuid.IsValid())
    {
        return false;
    }

	ProcessNodeByGuid(NextNodeGuid);

	return true;
}

FOpenLogicRuntimeNode UOpenLogicRuntimeEventContext::GetRuntimeNode(FGuid NodeGuid) const
{
	return RuntimeNodes[NodeGuid];
}

void UOpenLogicRuntimeEventContext::InitializeRuntimeNode(FGuid NodeGuid)
{
	if (!NodeGuid.IsValid() || !RuntimeNodes.Contains(NodeGuid))
	{
		return;
	}

}

bool UOpenLogicRuntimeEventContext::IsNodeProcessed(FGuid NodeGuid) const
{
	return RuntimeNodes.Contains(NodeGuid);
}

FOpenLogicRuntimeNode UOpenLogicRuntimeEventContext::GetOrCreateRuntimeNode(FGuid NodeGuid)
{
	if (!NodeGuid.IsValid())
	{
		return FOpenLogicRuntimeNode();
	}

	// Check if the runtime node already exists
	if (RuntimeNodes.Contains(NodeGuid))
	{
		return RuntimeNodes[NodeGuid];
	}

	FOpenLogicNode NodeData = GetGraphNodeData(NodeGuid);

	if (NodeData.TaskClass.IsNull())
	{
		return FOpenLogicRuntimeNode();
	}

	FOpenLogicRuntimeNode RuntimeNode;
	RuntimeNode.TaskClass = NodeData.TaskClass;
	RuntimeNode.TaskState = EOpenLogicTaskState::None;
	RuntimeNode.TaskInstance = NewObject<UOpenLogicTask>(this, NodeData.TaskClass.LoadSynchronous());
	RuntimeNode.NodeID = NodeGuid;

	if (RuntimeNode.TaskInstance)
	{
		RuntimeNode.TaskInstance->SetGuid(NodeGuid);
		//RuntimeNode.TaskInstance->SetEventRuntimeContext(this);
	}

	RuntimeNodes.Add(NodeGuid, RuntimeNode);
	
	return RuntimeNode;
}

FOpenLogicNode UOpenLogicRuntimeEventContext::GetGraphNodeData(FGuid NodeGuid) const
{
	if (!NodeGuid.IsValid() || !GetWorker()->GetGraphData().Nodes.Contains(NodeGuid))
	{
		return FOpenLogicNode();
	}

	return GetWorker()->GetGraphData().Nodes[NodeGuid];
}
