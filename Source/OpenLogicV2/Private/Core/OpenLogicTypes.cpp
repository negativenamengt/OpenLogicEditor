// Copyright 2024 - NegativeNameSeller

#include "Core/OpenLogicTypes.h"
#include "NativeGameplayTags.h"
#include "Async/Async.h"
#include "Tasks/OpenLogicTask.h"
#include "Utility/PayloadObject.h"
#include "Widgets/ExecutionPinBase.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_OpenLogicUserLibrary, "OpenLogicUserLibrary", "Parent tag for user-defined node libraries in OpenLogic.");

TSharedPtr<void> FOpenLogicDefaultValue::DeserializeToStruct(UScriptStruct* StructType)
{
	if (!StructType || SerializedData.IsEmpty())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("DeserializeToStruct: Invalid input."));
		return nullptr;
	}

	const int32 StructSize = StructType->GetStructureSize();
	void* StructMemory = FMemory::Malloc(StructSize, StructType->GetMinAlignment());
	FMemory::Memzero(StructMemory, StructSize);
	StructType->InitializeStruct(StructMemory);

	// Always use CopyScriptStruct — just like DeserializePropertyFromStack
	if (SerializedData.Num() == StructSize)
	{
		const void* SrcPtr = SerializedData.GetData();
		StructType->CopyScriptStruct(StructMemory, SrcPtr);
	}
	else
	{
		// As a fallback: structured archive (less reliable)
		FMemoryReader Reader(SerializedData, false);
		FStructuredArchiveFromArchive Ar(Reader);
		FStructuredArchive::FSlot Slot = Ar.GetSlot();
		StructType->SerializeItem(Slot, StructMemory, nullptr);

		if (Reader.IsError())
		{
			UE_LOG(OpenLogicLog, Error, TEXT("DeserializeToStruct: Fallback deserialization failed for %s."), *StructType->GetName());
			StructType->DestroyStruct(StructMemory);
			FMemory::Free(StructMemory);
			return nullptr;
		}
	}

	return TSharedPtr<void>(StructMemory, [StructType](void* Ptr)
	{
		StructType->DestroyStruct(Ptr);
		FMemory::Free(Ptr);
	});
}

TArray<FName> FOpenLogicPinData::GetRelevantPropertyAttributes() const
{
	TArray<FName> RelevantAttributes;

	if (!PropertyClass || Role != EPinRole::DataProperty)
	{
		return RelevantAttributes;
	}

	// Get the UEnum object for the EOpenLogicPinAttribute enum
	const UEnum* EnumPtr = StaticEnum<EOpenLogicPinAttribute>();
	if (!EnumPtr)
	{
		return RelevantAttributes;
	}

	const uint8 SupportedAttributes = PropertyClass.GetDefaultObject()->SupportedAttributes;
	
	// Iterate through all bits in the bitmask
	for (int32 i = 0; i < 8 * sizeof(SupportedAttributes); ++i)
	{
		// Create a bitmask for the current bit
		const int32 BitMask = 1 << i;

		// Check if this bit is set in the bitmask
		if (SupportedAttributes & BitMask)
		{
			// Get the name of the enum value corresponding to this bit
			const FString EnumName = EnumPtr->GetNameByValue(BitMask).ToString();

			// Add it to the list of relevant attributes
			RelevantAttributes.Add(FName(EnumName));
		}
	}

	return RelevantAttributes;
}

FOpenLogicPinData FOpenLogicNode::GetPinData(int32 PinIndex, bool bIsInput) const
{
	TMap<int32, FOpenLogicPinState> Pins = bIsInput ? InputPins : OutputPins;
	if (!Pins.Contains(PinIndex))
	{
		return FOpenLogicPinData();
	}

	const FOpenLogicPinState& PinState = Pins[PinIndex];
	if (PinState.IsUserCreated)
	{
		return PinState.PinData;
	}
	
	UClass* Class = TaskClass.LoadSynchronous();
	if (!Class)
	{
		return FOpenLogicPinData();
	}

	UOpenLogicTask* DefaultObject = Class->GetDefaultObject<UOpenLogicTask>();

	if (bIsInput)
	{
		return DefaultObject->GetInputPinData(PinIndex - 1);
	}

	return DefaultObject->GetOutputPinData(PinIndex - 1);
}

int32 FOpenLogicNode::GetPinIndexFromName(const FName& PinName, bool bIsInput) const
{
	const TMap<int32, FOpenLogicPinState> Pins = bIsInput ? InputPins : OutputPins;

	for (const auto& PinPair : Pins)
	{
		FOpenLogicPinData PinData = bIsInput ? GetInputPinData(PinPair.Key) : GetOutputPinData(PinPair.Key);

		if (PinData.PinName == PinName)
		{
			return PinPair.Key;
		}
	}

	return INDEX_NONE;
}

bool FOpenLogicGraphData::MigrateToLatestSchemaVersion()
{
	int32 PreviousSchemaVersion = SchemaVersion;
	
	if (SchemaVersion < 1)
	{
		Migration_TaskProperties();
		Migration_BlueprintContentSerialization();

		SchemaVersion = 1;
	}

	SchemaVersion = GetLatestSchemaVersion();

	return SchemaVersion != PreviousSchemaVersion;
}

void FOpenLogicGraphData::Migration_TaskProperties()
{
	if (SchemaVersion > 0)
	{
		return;
	}
	
	for (auto NodePair : Nodes)
	{
		FGuid NodeGuid = NodePair.Key;
		FOpenLogicNode& Node = NodePair.Value;

		TMap<FGuid, FString>& BlueprintContent = Node.BlueprintContent;
		if (BlueprintContent.Num() == 0)
		{
			continue;
		}

		// Force load the task soft class ptr
		TSubclassOf<UOpenLogicTask> TaskClass = Node.TaskClass.LoadSynchronous();
		if (!TaskClass)
		{
			continue;
		}

		// Access the default object of the task class
		UOpenLogicTask* DefaultObject = TaskClass.GetDefaultObject();
		if (!DefaultObject)
		{
			continue;
		}

		if (DefaultObject->GetBlueprintTaskProperties().IsEmpty())
		{
#if WITH_EDITOR
			DefaultObject->RefreshBlueprintTaskProperties();
			DefaultObject->MarkPackageDirty();
#endif
		}

		const TMap<FGuid, FName>& DeprecatedTaskProperties = DefaultObject->GetDeprecatedTaskProperties();

		for (auto ContentPair : BlueprintContent)
		{
			FGuid PropertyGuid = ContentPair.Key;
			FString PropertyName = ContentPair.Value;

			// Check if the property should be migrated
			if (!DeprecatedTaskProperties.Contains(PropertyGuid))
			{
				continue;
			}

			FGuid NewPropertyGuid = DefaultObject->GetPropertyIdentifierByName(DeprecatedTaskProperties[PropertyGuid]);

			// Replace the property guid with the new one
			BlueprintContent.Remove(PropertyGuid);
			BlueprintContent.Add(NewPropertyGuid, PropertyName);

			Nodes[NodeGuid] = Node;
		}
	}
}

void FOpenLogicGraphData::Migration_BlueprintContentSerialization()
{
	if (SchemaVersion >= 1)
	{
		return;
	}

	TMap<TSoftClassPtr<UOpenLogicTask>, UOpenLogicTask*> Cache_TaskInstances;
	TMap<TSoftClassPtr<UOpenLogicTask>, TMap<FGuid, FProperty*>> Cache_TaskProperties;

	for (auto NodePair : Nodes)
	{
		UOpenLogicTask* TaskObject = nullptr;
		if (Cache_TaskInstances.Contains(NodePair.Value.TaskClass))
		{
			TaskObject = Cache_TaskInstances[NodePair.Value.TaskClass];
		} else
		{
			TaskObject = NewObject<UOpenLogicTask>(GetTransientPackage(), NodePair.Value.TaskClass.LoadSynchronous());
			if (!TaskObject)
			{
				continue;
			}
			
			Cache_TaskInstances.Add(NodePair.Value.TaskClass, TaskObject);
		}

		if (!Cache_TaskProperties.Contains(NodePair.Value.TaskClass))
		{
			TMap<FGuid, FProperty*> TaskProperties;

			const TMap<FGuid, FName>& BlueprintTaskProperties = TaskObject->GetBlueprintTaskProperties();

			for (auto PropPair : BlueprintTaskProperties)
			{
				FProperty* Property = TaskObject->GetClass()->FindPropertyByName(PropPair.Value);
				if (!Property)
				{
					continue;
				}

				TaskProperties.Add(PropPair.Key, Property);
			}
				
			Cache_TaskProperties.Add(NodePair.Value.TaskClass, TaskProperties);
		}

		for (auto ContentPair : NodePair.Value.BlueprintContent)
		{
			// Get the cached property by GUID
			FProperty* Property = Cache_TaskProperties[NodePair.Value.TaskClass].FindRef(ContentPair.Key);
			if (!Property)
			{
				continue;
			}

			// Deserialize the property value
			bool bSuccess = false;
			UPayloadObject* Payload = UPayloadObject::Parse(ContentPair.Value, bSuccess);

			if (!bSuccess || !Payload)
			{
				continue;
			}

			// Use the deserialized payload to set the property value
			void* ValuePtr = Property->ContainerPtrToValuePtr<void>(TaskObject);
			UPayloadObject::CreateWildcardValue(Payload, Property, ValuePtr);

			// Reserialize with FProperty ExportText
			FString NewValue;
			Property->ExportTextItem_Direct(NewValue, ValuePtr, ValuePtr, nullptr, PPF_None, nullptr);

			if (NewValue.IsEmpty())
			{
				continue;
			}
			
			// Update the content
			Nodes[NodePair.Key].BlueprintContent[ContentPair.Key] = NewValue;
		}
	}
}

UOpenLogicTask* FOpenLogicTaskPool::GetTaskInstance(TSubclassOf<UOpenLogicTask> TaskClass, UObject* Outer)
{
	UOpenLogicTask* TaskInstance;

	// Check if there are any available tasks
	if (AvailableTasks.Num() > 0)
	{
		// Get the first available task
		TaskInstance = *AvailableTasks.CreateIterator();
		AvailableTasks.Remove(TaskInstance);
	} else
	{
		// Create a new task instance if none are available
		TaskInstance = NewObject<UOpenLogicTask>(Outer, TaskClass);

		if (!IsInGameThread())
		{
			AsyncTask(ENamedThreads::GameThread, [TaskInstance]
			{
				// Clear the async flag to prevent any GC issues
				TaskInstance->ClearInternalFlags(EInternalObjectFlags::Async);
			});
		}
	}
	
	// Add the task instance to the active tasks
	ActiveTasks.Add(TaskInstance);

	return TaskInstance;
}

void FOpenLogicTaskPool::ReturnTaskInstance(UOpenLogicTask* TaskInstance)
{
	if (!TaskInstance && !ActiveTasks.Contains(TaskInstance))
	{
		return;
	}

	ActiveTasks.Remove(TaskInstance);
	TaskInstance->ResetTaskState();
	AvailableTasks.Add(TaskInstance);
}

bool FOpenLogicDefaultValueHandle::CommitChange()
{
	switch (HandleType)
	{
#if WITH_EDITOR
	case EOpenLogicDefaultValueHandleType::PropertyHandle:
		{
			if (PropertyHandle && PropertyHandle.IsValid())
			{
				void* ValuePtr = nullptr;
				if (PropertyHandle->GetValueData(ValuePtr) == FPropertyAccess::Success && ValuePtr)
				{
					FOpenLogicDefaultValue* InternalValue = static_cast<FOpenLogicDefaultValue*>(ValuePtr);
					*InternalValue = CachedDefaultValue;

					PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
				}
			}
				
			break;	
		}
#endif
	case EOpenLogicDefaultValueHandleType::ExecutionPin:
		if (ExecutionPin)
		{
			ExecutionPin->SetDefaultValue(CachedDefaultValue);
		}
		break;

	default:
		break;
	}

	return false;
}

bool FOpenLogicDefaultValueHandle::IsValid() const
{
	switch (HandleType)
	{
#if WITH_EDITOR
	case EOpenLogicDefaultValueHandleType::PropertyHandle:
		return PropertyHandle.IsValid();
#endif
	case EOpenLogicDefaultValueHandleType::ExecutionPin:
		return ExecutionPin != nullptr && ExecutionPin->IsValidLowLevel();

	default:
		return false;
	}
}

FOpenLogicDefaultValue FOpenLogicDefaultValueHandle::GetNodeValue() const
{
	switch (HandleType)
	{
#if WITH_EDITOR
	case EOpenLogicDefaultValueHandleType::PropertyHandle:
		{
			if (PropertyHandle && PropertyHandle.IsValid())
			{
				void* ValuePtr = nullptr;
				if (PropertyHandle->GetValueData(ValuePtr) == FPropertyAccess::Success && ValuePtr)
				{
					if (FOpenLogicDefaultValue* Value = static_cast<FOpenLogicDefaultValue*>(ValuePtr))
					{
						return *Value;
					}
				}
			}
				
			break;	
		}
#endif
	case EOpenLogicDefaultValueHandleType::ExecutionPin:
		if (ExecutionPin)
		{
			return ExecutionPin->GetDefaultValue();
		}
		break;

	default:
		break;
	}

	return FOpenLogicDefaultValue();
}
