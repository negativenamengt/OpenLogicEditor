// Copyright 2024 - NegativeNameSeller

#include "Runtime/OpenLogicRuntimeGraph.h"
#include "Runtime/OpenLogicRuntimeEventContext.h"
#include "Runtime/OpenLogicGraphRunnable.h"
#include "Templates/SubclassOf.h"
#include "OpenLogicV2.h"
#include "Async/Async.h"
#include "Misc/OutputDeviceNull.h"

bool UOpenLogicRuntimeGraph::TriggerEvent(TSubclassOf<UOpenLogicTask> TaskClass, bool AutoProcess, FOpenLogicGraphExecutionHandle& OutExecutionHandle)
{
	TArray<FGuid> EventImplementations = GetEventImplementations(TaskClass);
	if (EventImplementations.IsEmpty())
	{
		OutExecutionHandle = FOpenLogicGraphExecutionHandle();
		return false;
	}

	TSharedPtr<FOpenLogicGraphExecutionHandle> EventExecutionHandle = CreateExecutionHandle(EventImplementations[0]);
	if (!EventExecutionHandle || !EventExecutionHandle->IsValid())
	{
		OutExecutionHandle = FOpenLogicGraphExecutionHandle();
		return false;
	}

	if (AutoProcess)
	{
		ProcessExecutionHandle(EventExecutionHandle);
	}

	OutExecutionHandle = *EventExecutionHandle;
	return true;
}

TArray<FOpenLogicGraphExecutionHandle> UOpenLogicRuntimeGraph::TriggerAllEvents(TSubclassOf<UOpenLogicTask> TaskClass, bool AutoProcess)
{
	TArray<FOpenLogicGraphExecutionHandle> EventExecutionHandles;

	for (FGuid NodeID : GetEventImplementations(TaskClass))
	{
		TSharedPtr<FOpenLogicGraphExecutionHandle> EventExecutionHandle = CreateExecutionHandle(NodeID);
		if (!EventExecutionHandle || !EventExecutionHandle->IsValid())
		{
			continue;
		}

		if (AutoProcess)
        {
            ProcessExecutionHandle(EventExecutionHandle);
        }
		
		EventExecutionHandles.Add(*EventExecutionHandle);
	}

	return EventExecutionHandles;
}

TSharedPtr<FOpenLogicGraphExecutionHandle> UOpenLogicRuntimeGraph::CreateExecutionHandle(FGuid NodeID)
{
	if (!NodeID.IsValid() || !GetGraphData().Nodes.Contains(NodeID))
	{
		return nullptr;
	}

	TSoftClassPtr<UOpenLogicTask> TaskClass = GetNodeData(NodeID).TaskClass;

	if (!TaskClass.IsValid())
	{
		return nullptr;
	}

	TSharedPtr<FOpenLogicGraphExecutionHandle> NewHandle = MakeShared<FOpenLogicGraphExecutionHandle>();
	NewHandle->HandleIndex = GetNextHandleIndex();
	NewHandle->TaskClass = TaskClass;
	NewHandle->NodeID = NodeID;
	NewHandle->RuntimeGraph = this;

	HandleRegistry.Add(NewHandle->HandleIndex, NewHandle);

	return NewHandle;
}

TSharedPtr<FOpenLogicGraphExecutionHandle> UOpenLogicRuntimeGraph::GetExecutionHandle(int32 HandleIndex) const
{
	if (!HandleRegistry.Contains(HandleIndex))
	{
		return nullptr;
	}

	return HandleRegistry[HandleIndex];
}

void UOpenLogicRuntimeGraph::DestroyExecutionHandle(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (!ExecutionHandle.IsValid())
	{
		return;
	}

	// Iterate over each runtime node in the execution handle
	for (auto& NodePair : ExecutionHandle->RuntimeNodes)
	{
		TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = NodePair.Value;
		if (!RuntimeNode.IsValid())
		{
			continue;
		}

		UOpenLogicTask* TaskInstance = RuntimeNode->TaskInstance;
		if (IsValid(TaskInstance))
		{
			// Cancel any latent actions if the task hasn't completed
			if (RuntimeNode->TaskState != EOpenLogicTaskState::Completed)
			{
				if (UWorld* World = GetWorld())
				{
					World->GetLatentActionManager().RemoveActionsForObject(TaskInstance);
				}
				TaskInstance->OnTaskCompleted();	
			}

			// If the task is not persistent, return it to the pool
			if (RuntimeNode->TaskInstance->NodeLifecycle != ENodeLifecycle::Persistent)
			{
				FOpenLogicTaskPool& TaskPool = TaskPools.FindOrAdd(RuntimeNode->TaskInstance->GetClass());
				TaskPool.ReturnTaskInstance(TaskInstance);
			}
		}

		// Clear out stored properties
		RuntimeNode->InputProperties.Empty();
		RuntimeNode->OutputProperties.Empty();
		RuntimeNode->TaskInstance = nullptr;
	}

	HandleRegistry.Remove(ExecutionHandle->HandleIndex);
	ExecutionHandle->RuntimeNodes.Empty();
}

void UOpenLogicRuntimeGraph::BP_DestroyExecutionHandle(FOpenLogicGraphExecutionHandle ExecutionHandle)
{
	if (!ExecutionHandle.IsValid())
	{
		return;
	}
	
	TSharedPtr<FOpenLogicGraphExecutionHandle> Handle = GetExecutionHandle(ExecutionHandle.HandleIndex);
	if (Handle.IsValid())
	{
		DestroyExecutionHandle(Handle);
	}
}

TArray<FOpenLogicGraphExecutionHandle> UOpenLogicRuntimeGraph::GetExecutionHandles() const
{
	TArray<FOpenLogicGraphExecutionHandle> Handles;
	Handles.Reserve(HandleRegistry.Num());

	for (auto& HandlePair : HandleRegistry)
	{
		Handles.Add(*HandlePair.Value);
	}

	return Handles;
}

TSharedPtr<FOpenLogicRuntimeNode> UOpenLogicRuntimeGraph::ProcessNodeByGUID(FGuid NodeID, TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle)
{
	if (!NodeID.IsValid() || !ExecutionHandle.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ProcessNodeByGUID] Invalid NodeID or ExecutionHandle."));
		return nullptr;
	}
	
	TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = GetOrCreateRuntimeNode(NodeID, ExecutionHandle);
	if (!RuntimeNode.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ProcessNodeByGUID] Failed to create or retrieve RuntimeNode."));
		return nullptr;
	}

	FOpenLogicNode NodeData = GetNodeData(NodeID);

	// Initialize the runtime node
	RuntimeNode->InputPinsCount = NodeData.InputPins.Num();
	RuntimeNode->OutputPinsCount = NodeData.OutputPins.Num();

	return RuntimeNode;
}

void UOpenLogicRuntimeGraph::ActivateNode(TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode, const FName& PinName)
{
	if (!RuntimeNode.IsValid() || !RuntimeNode->TaskInstance)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ActivateNode] Invalid RuntimeNode or TaskInstance."));
		return;
	}

	UOpenLogicTask* TaskInstance = RuntimeNode->TaskInstance;

	if (!IsValid(TaskInstance))
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ActivateNode] Invalid TaskInstance."));
		return;
	}

	RuntimeNode->TaskState = EOpenLogicTaskState::Running;

	const TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle = FindExecutionHandleForTask(TaskInstance);
	if (!ExecutionHandle.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ActivateNode] ExecutionHandle not found for TaskInstance."));
		return;
	}

	PreloadInputPropertiesForNode(RuntimeNode, ExecutionHandle);

	// Call the OnNodeActivated runtime graph delegate
	OnNodeActivated.Broadcast(TaskInstance);

	TaskInstance->OnTaskActivated(GetContext(), PinName);
}

void UOpenLogicRuntimeGraph::CompleteNode(UOpenLogicTask* TaskInstance)
{
	if (!IsValid(TaskInstance))
    {
        return;
    }

	TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = FindRuntimeNodeForTask(TaskInstance);
	if (!RuntimeNode.IsValid())
    {
        return;
    }

	// Call the OnNodeCompleted runtime graph delegate
	OnNodeCompleted.Broadcast(RuntimeNode->TaskInstance);

	// Call the OnTaskCompleted event
	RuntimeNode->TaskInstance->OnTaskCompleted();
	
	// Set the task state to completed
	RuntimeNode->TaskState = EOpenLogicTaskState::Completed;

	// Cancel any latent actions
	if (UWorld* World = GetWorld())
	{
		World->GetLatentActionManager().RemoveActionsForObject(RuntimeNode->TaskInstance);
	}
	
	// Return the task instance to the pool
	FOpenLogicTaskPool& TaskPool = TaskPools.FindOrAdd(RuntimeNode->TaskInstance->GetClass());

	if (RuntimeNode->TaskInstance->NodeLifecycle != ENodeLifecycle::Persistent)
	{
		TaskPool.ReturnTaskInstance(RuntimeNode->TaskInstance);
		RuntimeNode->TaskInstance = nullptr;
	}
}

bool UOpenLogicRuntimeGraph::Then(UOpenLogicTask* TaskInstance, int32 NextPinIndex)
{
	if (!IsValid(TaskInstance))
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] Invalid TaskInstance."));
		return false;
	}

	TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle = FindExecutionHandleForTask(TaskInstance);
	if (!ExecutionHandle.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] %s: ExecutionHandle not found."), *TaskInstance->GetName());
		return false;
	}

	const FGuid NodeID = TaskInstance->GetGuid();
	if (!NodeID.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] %s: NodeID not valid."), *TaskInstance->GetName());
		return false;
	}

	TSharedPtr<FOpenLogicRuntimeNode>* RuntimeNode = ExecutionHandle->RuntimeNodes.Find(NodeID);
	if (!RuntimeNode || !RuntimeNode->IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] %s: RuntimeNode not found."), *TaskInstance->GetName());
		return false;
	}

	FOpenLogicNode NodeData = GetNodeData(NodeID);

	const FOpenLogicPinState* PinState = NodeData.OutputPins.Find(NextPinIndex);
	if (!PinState || PinState->Connections.IsEmpty())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] %s: PinState not found or has no connections."), *TaskInstance->GetName());
		return false;
	}

	const FGuid NextNodeGuid = PinState->Connections[0].NodeID;
	if (!NextNodeGuid.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[Then] %s: NextNodeGuid not valid."), *TaskInstance->GetName());
		return false;
	}

	TSharedPtr<FOpenLogicRuntimeNode> NextRuntimeNode = ProcessNodeByGUID(NextNodeGuid, ExecutionHandle);
	if (!NextRuntimeNode.IsValid())
	{
		return false;
	}

	FOpenLogicNode NextNodeData = GetNodeData(NextNodeGuid);
	ActivateNode(NextRuntimeNode, NextNodeData.GetInputPinData(PinState->Connections[0].PinID).PinName);
	return true;
}

void UOpenLogicRuntimeGraph::SetDataPropertyValue(UOpenLogicTask* TaskInstance, FName PinName, const TSharedPtr<void>& Value) const
{
	if (!IsValid(TaskInstance) || !PinName.IsValid() || !Value.IsValid())
	{
		return;
	}

	const TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = FindRuntimeNodeForTask(TaskInstance);
	if (!RuntimeNode.IsValid() || RuntimeNode->TaskState != EOpenLogicTaskState::Running)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[SetDataPropertyValue] Invalid RuntimeNode or TaskState."));
		return;
	}

	const FOpenLogicNode& NodeData = GetNodeData(RuntimeNode->NodeID);
	const int32 PinIndex = NodeData.GetOutputPinIndexFromName(PinName);

	if (PinIndex == INDEX_NONE)
	{
		UE_LOG(OpenLogicLog, Warning, TEXT("[SetDataPropertyValue] PinName %s not found."), *PinName.ToString());
		return;
	}

	RuntimeNode->OutputProperties.Add(PinIndex, Value);
}

void UOpenLogicRuntimeGraph::SetDataPropertyValueByAddress(UOpenLogicTask* TaskInstance, FName PinName, FProperty* Property, void* SourceAddress) const
{
	if (!IsValid(TaskInstance) || !PinName.IsValid() || !Property || !SourceAddress)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[SetDataPropertyValueByAddress] Invalid parameters."));
		return;
	}

	// Allocate memory and copy the property value
	void* CopiedValue = FMemory::Malloc(Property->GetElementSize() * Property->ArrayDim);
	FMemory::Memzero(CopiedValue, Property->GetElementSize() * Property->ArrayDim);
	Property->InitializeValue(CopiedValue);
	Property->CopyCompleteValue(CopiedValue, SourceAddress);

	// Wrap in a TSharedPtr with correct destroy callback
	TSharedPtr<void> Value(CopiedValue, [Property](void* Ptr)
	{
		if (Property && Ptr)
		{
			Property->DestroyValue(Ptr);
		}
		FMemory::Free(Ptr);
	});

	SetDataPropertyValue(TaskInstance, PinName, Value);
}

TSharedPtr<void> UOpenLogicRuntimeGraph::GetDataPropertyValue(UOpenLogicTask* TaskInstance, FName PinName)
{
	if (!IsValid(TaskInstance) || !PinName.IsValid())
	{
		return nullptr;
	}

	const TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = FindRuntimeNodeForTask(TaskInstance);
	if (!RuntimeNode.IsValid() || RuntimeNode->TaskState != EOpenLogicTaskState::Running)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[GetDataPropertyValue] Invalid RuntimeNode or TaskState."));
		return nullptr;
	}

	const FOpenLogicNode& NodeData = GetNodeData(RuntimeNode->NodeID);
	const int32 PinIndex = NodeData.GetInputPinIndexFromName(PinName);

	if (PinIndex == INDEX_NONE)
	{
		return nullptr;
	}

	TSharedPtr<void> FoundValue = RuntimeNode->InputProperties.FindRef(PinIndex);
	if (!FoundValue.IsValid())
	{
		return nullptr;
	}

	return FoundValue;
}

void UOpenLogicRuntimeGraph::PreloadInputPropertiesForNode(const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (!RuntimeNode.IsValid() || !ExecutionHandle.IsValid())
	{
		return;
	}

	RuntimeNode->InputProperties.Empty();

	const FOpenLogicNode& NodeData = GetNodeData(RuntimeNode->NodeID);

	for (const auto& InputPinPair : NodeData.InputPins)
	{
		const int32 InputPinIndex = InputPinPair.Key;
		const FOpenLogicPinState& InputPinState = InputPinPair.Value;

		const FOpenLogicPinData& PinData = NodeData.GetInputPinData(InputPinIndex);
		if (PinData.Role != EPinRole::DataProperty)
		{
			continue;
		}

		TSharedPtr<void> PropertyValue;

		if (InputPinState.Connections.Num() > 0)
		{
			PropertyValue = ResolveConnectedPinValue(RuntimeNode, ExecutionHandle, InputPinState.Connections[0]);
		}

		// Fallback to default value if no connection is found
		if (!PropertyValue.IsValid())
		{
			if (const UOpenLogicProperty* PropertyInstance = Cast<UOpenLogicProperty>(PinData.PropertyClass->GetDefaultObject()))
			{
				PropertyValue = CreatePropertyValueFromDefault(InputPinState.DefaultValue, PropertyInstance);
			}
		}

		if (PropertyValue.IsValid())
		{
			RuntimeNode->InputProperties.Add(InputPinIndex, PropertyValue);
		}
	}
}

TSharedPtr<void> UOpenLogicRuntimeGraph::ResolveConnectedPinValue(const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle, const FOpenLogicPinConnection& Connection)
{
	if (!RuntimeNode.IsValid() || !ExecutionHandle.IsValid() || !Connection.NodeID.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ResolveConnectedPinValue] Invalid parameters."));
		return nullptr;
	}

	bool bConnectionNodeFound = ExecutionHandle->RuntimeNodes.Contains(Connection.NodeID);

	TSharedPtr<FOpenLogicRuntimeNode> ConnectionRuntimeNode = ProcessNodeByGUID(Connection.NodeID, ExecutionHandle);
	if (!ConnectionRuntimeNode.IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ResolveConnectedPinValue] ConnectionRuntimeNode not valid."));
		return nullptr;
	}

	if (!bConnectionNodeFound || ConnectionRuntimeNode->bReevaluateOnDemand)
	{
		UOpenLogicTask* TaskInstance = ConnectionRuntimeNode->TaskInstance;
		if (!TaskInstance)
		{
			ConnectionRuntimeNode->TaskInstance = GetOrCreateTaskInstance(ConnectionRuntimeNode, ExecutionHandle);
			InitializeTaskInstance(ConnectionRuntimeNode->TaskInstance, ConnectionRuntimeNode, ExecutionHandle);
		}

		ActivateNode(ConnectionRuntimeNode, NAME_None);
	}

	if (const TSharedPtr<void>* FoundValue = ConnectionRuntimeNode->OutputProperties.Find(Connection.PinID))
	{
		return *FoundValue;
	}
	return nullptr;
}

TSharedPtr<void> UOpenLogicRuntimeGraph::CreatePropertyValueFromDefault(const FOpenLogicDefaultValue& DefaultValue, const UOpenLogicProperty* PropertyInstance)
{
	if (!PropertyInstance)
	{
		return nullptr;
	}

	switch (PropertyInstance->UnderlyingType)
	{
		case EOpenLogicUnderlyingType::Boolean:
		{
			bool Value = false;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<bool>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Byte:
		{
			uint8 Value = 0;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<uint8>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Int:
		{
			int32 Value = 0;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<int32>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Float:
		{
			float Value = 0.0f;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<float>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Double:
		{
			double Value = 0.0;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<double>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::String:
		{
			FString Value;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<FString>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Name:
		{
			FName Value;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<FName>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Text:
		{
			FText Value;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<FText>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Object:
		case EOpenLogicUnderlyingType::Class:
		{
			UObject* Value = nullptr;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<UObject*>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Enum:
		{
			int32 Value = 0;
			if (DefaultValue.GetValue(Value))
			{
				return MakeShared<int32>(Value);
			}
			break;
		}
		case EOpenLogicUnderlyingType::Struct:
		{
			if (!PropertyInstance || !PropertyInstance->StructType)
			{
				UE_LOG(OpenLogicLog, Error, TEXT("StructType is not set for property."));
				break;
			}

			FOpenLogicDefaultValue StructValue = DefaultValue;

			return StructValue.DeserializeToStruct(PropertyInstance->StructType);
		}
		case EOpenLogicUnderlyingType::Wildcard:
		{
			// Wildcard — not enough info to safely deserialize
			UE_LOG(OpenLogicLog, Error, TEXT("Wildcard type cannot be deserialized."));
			break;
		}
	}

	return nullptr;
}

FOpenLogicDefaultValue UOpenLogicRuntimeGraph::GetDefaultValue(UOpenLogicTask* TaskInstance, FName PinName)
{
	TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = FindRuntimeNodeForTask(TaskInstance);
	if (!RuntimeNode.IsValid() || !PinName.IsValid())
	{
		return FOpenLogicDefaultValue();
	}

	FOpenLogicNode NodeData = GetNodeData(RuntimeNode->NodeID);
	int32 InputPinIndex = NodeData.GetInputPinIndexFromName(PinName);

	if (InputPinIndex == -1 || !NodeData.InputPins.Contains(InputPinIndex))
	{
		return FOpenLogicDefaultValue();
	}

	return NodeData.InputPins[InputPinIndex].DefaultValue;
}

TSharedPtr<FOpenLogicGraphExecutionHandle> UOpenLogicRuntimeGraph::FindExecutionHandleForTask(const UOpenLogicTask* TaskInstance) const
{
	if (!TaskInstance || TaskInstance->GetExecutionHandleIndex() == INDEX_NONE)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[FindExecutionHandleForTask] Invalid TaskInstance or ExecutionHandleIndex."));
		return nullptr;
	}

	int32 HandleIndex = TaskInstance->GetExecutionHandleIndex();

	const TSharedPtr<FOpenLogicGraphExecutionHandle>* FoundHandle = HandleRegistry.Find(HandleIndex);
	if (!FoundHandle || !FoundHandle->IsValid())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[FindExecutionHandleForTask] ExecutionHandle not found for TaskInstance %s."), *TaskInstance->GetName());
		return nullptr;
	}

	return *FoundHandle;
}

TSharedPtr<FOpenLogicRuntimeNode> UOpenLogicRuntimeGraph::FindRuntimeNodeForTask( const UOpenLogicTask* TaskInstance) const
{
	if (!TaskInstance)
	{
		return nullptr;
	}

	TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle = FindExecutionHandleForTask(TaskInstance);
	if (!ExecutionHandle.IsValid())
	{
		return nullptr;
	}

	if (ExecutionHandle->RuntimeNodes.Contains(TaskInstance->GetGuid()))
	{
		return ExecutionHandle->RuntimeNodes[TaskInstance->GetGuid()];
	}

	return nullptr;
}

void UOpenLogicRuntimeGraph::ProcessExecutionHandle(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (!ExecutionHandle.IsValid() || ExecutionHandle->IsProcessed)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[ProcessExecutionHandle] Invalid Handle."));
		return;
	}

	// Set the execution handle as processed and running
	ExecutionHandle->IsProcessed = true;
	ExecutionHandle->IsRunning = true;

	// Run the entry node
	if (GetThreadSettings().NodeExecutionThread == EOpenLogicRuntimeThreadType::GameThread || !IsInGameThread())
	{
		TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = GetOrCreateRuntimeNode(ExecutionHandle->NodeID, ExecutionHandle);
		if (!RuntimeNode.IsValid())
		{
			return;
		}

		ActivateNode(RuntimeNode, NAME_None);
	} else
	{
		AddExecutionHandleToQueue(ExecutionHandle);
	}
}

bool UOpenLogicRuntimeGraph::IsExecutionHandleValid(const FOpenLogicGraphExecutionHandle& ExecutionHandle)
{
	return ExecutionHandle.IsValid();
}

TArray<FGuid> UOpenLogicRuntimeGraph::GetEventImplementations(TSubclassOf<UOpenLogicTask> TaskClass) const
{
	FOpenLogicGraphData GraphData = GetGraphData();
	if (!GraphData.Events.Contains(TaskClass))
	{
		return TArray<FGuid>();
	}

	return GraphData.Events[TaskClass].NodeId;
}

TSharedPtr<FOpenLogicRuntimeNode> UOpenLogicRuntimeGraph::GetOrCreateRuntimeNode(FGuid NodeID, TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle)
{
	if (!NodeID.IsValid() || !ExecutionHandle.IsValid())
	{
		return nullptr;
	}

	// Check if the runtime node already exists
	if (TSharedPtr<FOpenLogicRuntimeNode>* ExistingRuntimeNode = ExecutionHandle->RuntimeNodes.Find(NodeID))
	{
		return *ExistingRuntimeNode;
	}

	FOpenLogicNode NodeData = GetNodeData(NodeID);

	if (NodeData.TaskClass.IsNull())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[GetOrCreateRuntimeNode] Invalid TaskClass for NodeID %s."), *NodeID.ToString());
		return nullptr;
	}

	TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = MakeShared<FOpenLogicRuntimeNode>();
	RuntimeNode->TaskClass = NodeData.TaskClass;
	RuntimeNode->TaskState = EOpenLogicTaskState::None;
	RuntimeNode->NodeID = NodeID;
	RuntimeNode->TaskInstance = GetOrCreateTaskInstance(RuntimeNode, ExecutionHandle);
	RuntimeNode->bReevaluateOnDemand = RuntimeNode->TaskInstance->ReevaluateOnDemand;
	InitializeTaskInstance(RuntimeNode->TaskInstance, RuntimeNode, ExecutionHandle);

	ExecutionHandle->RuntimeNodes.Add(NodeID, RuntimeNode);

	return RuntimeNode;
}
 
UOpenLogicTask* UOpenLogicRuntimeGraph::GetOrCreateTaskInstance(TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (!RuntimeNode.IsValid() || !ExecutionHandle.IsValid())
	{
		return nullptr;
	}

	if (RuntimeNode->TaskInstance)
	{
		return RuntimeNode->TaskInstance;
	}

	TSubclassOf<UOpenLogicTask> TaskClass = RuntimeNode->TaskClass.Get();
	if (!TaskClass)
	{
		return nullptr;
	}

	if (PersistentNodes.Contains(RuntimeNode->NodeID))
	{
		RuntimeNode->TaskInstance = PersistentNodes[RuntimeNode->NodeID];
		return RuntimeNode->TaskInstance;
	}

	// Get the task instance from the pool
	FOpenLogicTaskPool& TaskPool = TaskPools.FindOrAdd(TaskClass);

	UOpenLogicTask* TaskInstance = TaskPool.GetTaskInstance(TaskClass, this);

	if (TaskInstance->NodeLifecycle == ENodeLifecycle::Persistent)
	{
		PersistentNodes.Add(RuntimeNode->NodeID, TaskInstance);
	}

	return TaskInstance;
}

void UOpenLogicRuntimeGraph::InitializeTaskInstance(UOpenLogicTask* TaskInstance, const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (!TaskInstance || !RuntimeNode.IsValid() || !ExecutionHandle.IsValid())
	{
		return;
	}

	TaskInstance->SetGuid(RuntimeNode->NodeID);
	TaskInstance->SetRuntimeGraph(this);
	TaskInstance->SetExecutionHandleIndex(ExecutionHandle->HandleIndex);

	// Loading task properties
	FOpenLogicNode NodeData = GetNodeData(RuntimeNode->NodeID);

	for (const TPair<FGuid, FString> BlueprintContent : NodeData.BlueprintContent)
	{
		FProperty* Property = FindFProperty<FProperty>(TaskInstance->GetClass(), TaskInstance->GetPropertyNameByGUID(BlueprintContent.Key));
		if (!Property)
		{
			continue;
		}
		
		// Capture any import errors
		FOutputDeviceNull ErrorText;
	
		Property->ImportText_Direct(*BlueprintContent.Value, Property->ContainerPtrToValuePtr<void>(TaskInstance), this, PPF_None, &ErrorText);
	}

	for (const TPair<FName, FString> CppContent : NodeData.CppContent)
	{
		FProperty* Property = FindFProperty<FProperty>(TaskInstance->GetClass(), CppContent.Key);
		if (!Property)
		{
			continue;
		}

		// Capture any import errors
		FOutputDeviceNull ErrorText;

		Property->ImportText_Direct(*CppContent.Value, Property->ContainerPtrToValuePtr<void>(TaskInstance), this, PPF_None, &ErrorText);
	}

}

/* Executes all the events associated with a specific Task class */
void UOpenLogicRuntimeGraph::MultiExecuteEvent(TSubclassOf<UOpenLogicTask> TaskClass, bool& bSuccess)
{
	FOpenLogicGraphExecutionHandle EventExecutionHandle;
	bSuccess = TriggerEvent(TaskClass, true, EventExecutionHandle);
}

void UOpenLogicRuntimeGraph::SetGraphData(FOpenLogicGraphData NewData)
{
	WorkerGraphData = NewData;

	for (auto& NodePair : WorkerGraphData.Nodes)
	{
		TSoftClassPtr<UOpenLogicTask> TaskClass = NodePair.Value.TaskClass;
		if (TaskClass.IsNull())
		{
			continue;
		}
		
		TaskClass.LoadSynchronous();
	}
}

FOpenLogicNode UOpenLogicRuntimeGraph::GetNodeData(FGuid NodeID) const
{
	if (!NodeID.IsValid() || !GetGraphData().Nodes.Contains(NodeID))
	{
		return FOpenLogicNode();
	}
	
	return GetGraphData().Nodes[NodeID];
}

void UOpenLogicRuntimeGraph::SetThreadSettings(FOpenLogicThreadSettings& NewThreadSettings)
{
	// Clean up the current thread before setting a new one
	CleanupThread();

	// Checks if the platform supports multithreading
	if (!FPlatformProcess::SupportsMultithreading())
	{
		NewThreadSettings.NodeExecutionThread = EOpenLogicRuntimeThreadType::GameThread;
		return;
	}
	
	ThreadSettings = NewThreadSettings;

	if (ThreadSettings.NodeExecutionThread == EOpenLogicRuntimeThreadType::BackgroundThread)
	{
		// Create the thread
		Runnable = new FOpenLogicGraphRunnable(this);
		RunnableThread = FRunnableThread::Create(Runnable, TEXT("OpenLogicGraphThread"), 0, TPri_Normal);

		UE_LOG(OpenLogicLog, Log, TEXT("Successfully launched runtime graph thread - %d"), RunnableThread->GetThreadID());
	}
}

void UOpenLogicRuntimeGraph::SetContext(UObject* NewContextObject)
{
	ContextObject = NewContextObject;
}

bool UOpenLogicRuntimeGraph::AddExecutionHandleToQueue(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle)
{
	if (ThreadSettings.NodeExecutionThread != EOpenLogicRuntimeThreadType::BackgroundThread || !Runnable)
	{
		return false;
	}

	Runnable->AddExecutionHandle(FOpenLogicQueuedExecutionHandle{ExecutionHandle->HandleIndex});

	return true;
}

void UOpenLogicRuntimeGraph::DestroyWorker()
{
	CleanupThread();

	TArray<TSharedPtr<FOpenLogicGraphExecutionHandle>> HandlesToDestroy;
	HandleRegistry.GenerateValueArray(HandlesToDestroy);
	for (auto& Handle : HandlesToDestroy)
	{
		DestroyExecutionHandle(Handle);
	}

	HandleRegistry.Empty();
	PersistentNodes.Empty();
}

void UOpenLogicRuntimeGraph::CleanupThread()
{
	if (RunnableThread)
	{
		// Stop the thread
		if (Runnable)
		{
			Runnable->Stop();
		}
		
		// Kill the thread and make sure it completed its work
		RunnableThread->Kill(true);

		// Clean up the thread object
		delete RunnableThread;
		RunnableThread = nullptr;

		// Clean up the runnable object
		delete Runnable;
		Runnable = nullptr;
	}
}
