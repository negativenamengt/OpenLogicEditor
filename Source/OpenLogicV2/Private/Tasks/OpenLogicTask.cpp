// Copyright 2024 - NegativeNameSeller

#include "Tasks/OpenLogicTask.h"
#include "Widgets/NodeBase.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "OpenLogicV2.h"
#include "Async/Async.h"
#include "Engine/NetDriver.h"

#if WITH_EDITOR
    #include "Subsystems/AssetEditorSubsystem.h"
    #include "EdGraph/EdGraphNode.h"
#endif


UWorld* UOpenLogicTask::GetWorld() const
{
    if (HasAllFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    return GetOuter()->GetWorld();
}

bool UOpenLogicTask::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
    AActor* Owner = Cast<AActor>(GetOuter());
    if (!Owner) return false;
    UNetDriver* NetDriver = Owner->GetNetDriver();
    if (!NetDriver) return false;

    NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);

    return true;
}

int32 UOpenLogicTask::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
    if (AActor* OuterActor = Cast<AActor>(GetOuter()))
    {
        return OuterActor->GetFunctionCallspace(Function, Stack);
    }

    return FunctionCallspace::Local;
}

TMap<FGuid, FName> UOpenLogicTask::GetBlueprintTaskProperties(bool bIncludeSuper) const
{
    if (!bIncludeSuper)
    {
        return BlueprintTaskProperties;
    }
    
    TMap<FGuid, FName> AllProperties = BlueprintTaskProperties;

    // Iterate through all super classes
    for (UClass* SuperClass = GetClass()->GetSuperClass(); SuperClass; SuperClass = SuperClass->GetSuperClass())
    {
        if (SuperClass->IsChildOf(UOpenLogicTask::StaticClass()) && SuperClass->GetDefaultObject())
        {
            UOpenLogicTask* SuperTask = Cast<UOpenLogicTask>(SuperClass->GetDefaultObject());
            AllProperties.Append(SuperTask->GetBlueprintTaskProperties());
        }
    }
    
    return AllProperties;
}

TArray<FName> UOpenLogicTask::GetCPPTaskProperties() const
{
    TArray<FName> Properties;

    // Loop through all properties
    for (TFieldIterator<FProperty> PropIt(GetClass(), EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
    {
        // Get the property
        FProperty* Property = *PropIt;

        // Get the owner class of the property
        UClass* OwnerClass = Property->GetOwnerClass();

        // Check if the property has not the ExposeOnSpawn flag
        if (!Property->HasAnyPropertyFlags(CPF_ExposeOnSpawn))
        {
            continue;
        }

        if (OwnerClass->IsInBlueprint())
        {
            continue;
        }
        
        Properties.Add(Property->GetFName());
    }

    return Properties;
}

const FName UOpenLogicTask::GetPropertyNameByGUID(FGuid PropertyGUID)
{
    if (BlueprintTaskProperties.Contains(PropertyGUID))
    {
		return BlueprintTaskProperties[PropertyGUID];
	}

	return NAME_None;
}

const FGuid UOpenLogicTask::GetPropertyIdentifierByName(FName PropertyName, bool bIncludeSuper)
{
    // Check if the PropertyName is valid
    if (PropertyName.IsNone())
    {
        UE_LOG(OpenLogicLog, Warning, TEXT("[UOpenLogicTask::GetPropertyIdentifierByName] PropertyName is invalid."));
        return FGuid();
    }

    // Check if the property exists in the class
    const FProperty* Property = GetClass()->FindPropertyByName(PropertyName);
    if (!Property)
    {
        UE_LOG(OpenLogicLog, Warning, TEXT("[UOpenLogicTask::GetPropertyIdentifierByName] Property %s does not exist in class %s."), *PropertyName.ToString(), *GetClass()->GetName());
        return FGuid();
    }

    for (const auto& Elem : GetBlueprintTaskProperties(bIncludeSuper))
	{
		if (Elem.Value == PropertyName)
		{
			return Elem.Key;
		}
	}

	return FGuid();
}

bool UOpenLogicTask::DoesPropertyIdentifierExist(FGuid PropertyGUID)
{
    return BlueprintTaskProperties.Contains(PropertyGUID);
}

bool UOpenLogicTask::IsPropertyStored(FName PropertyName)
{
    for (const auto& Elem : BlueprintTaskProperties)
	{
		if (Elem.Value == PropertyName)
		{
			return true;
		}
	}

	return false;
}

bool UOpenLogicTask::IsCppProperty(FName PropertyName) const
{
    if (PropertyName.IsNone())
    {
        return false;
    }

    const FProperty* Property = GetClass()->FindPropertyByName(PropertyName);

    if (!Property || !Property->GetOwnerClass())
    {
        return false;
    }

    return !Property->GetOwnerClass()->IsInBlueprint();
}

TMap<FGuid, FName> UOpenLogicTask::GetDeprecatedTaskProperties()
{
    return TaskProperties;
}

/* Completes a task and performs necessary cleanup. */
void UOpenLogicTask::CompleteTask(FOpenLogicPinHandle PinName)
{
    if (!GetRuntimeGraph())
    {
        return;
    }

    // Cache the node data and execution handle
    FOpenLogicNode NodeData = GetRuntimeGraph()->GetNodeData(GetGuid());

    // Transition to the next node
    if (PinName.PinName != NAME_None)
    {
        int32 NextPinIndex = NodeData.GetOutputPinIndexFromName(PinName.PinName);
        if (NextPinIndex == INDEX_NONE)
        {
            return;
        }

        GetRuntimeGraph()->Then(this, NextPinIndex);
    }
    
    if (NodeLifecycle != ENodeLifecycle::Persistent)
    {
        GetRuntimeGraph()->CompleteNode(this);
    }
}

void UOpenLogicTask::ExecuteAndCompleteTask(int32 PinIndex)
{
	if (!GetRuntimeGraph())
	{
		return;
	}

    this->ExecutePin(PinIndex);
    this->CompleteTask();
}

/* Deprecated: Triggers a specific output pin to execute sequences of tasks. */
void UOpenLogicTask::ReturnTask(int ReturnPin)
{
    if (!GetRuntimeGraph())
    {
        return;
    }
    
    ExecuteAndCompleteTask(ReturnPin);
}

DEFINE_FUNCTION(UOpenLogicTask::execBP_SetPropertyValueByAttribute)
{
    Stack.MostRecentProperty = nullptr;

    // Get the pin attribute
    FOpenLogicPinHandle PinAttribute;
    Stack.Step(Stack.Object, &PinAttribute);

    // Get the value
    Stack.Step(Stack.Object, NULL);
    FProperty* ValueProp = CastField<FProperty>(Stack.MostRecentProperty);

    P_FINISH;
    P_NATIVE_BEGIN;
    
    if (P_THIS->RuntimeGraph)
    {
        void* ValueAddress = Stack.MostRecentPropertyAddress;
        P_THIS->GetRuntimeGraph()->SetDataPropertyValueByAddress(P_THIS, PinAttribute.PinName, ValueProp, ValueAddress);
    }

    P_NATIVE_END;
}

DEFINE_FUNCTION(UOpenLogicTask::execBP_GetPropertyValueByAttribute)
{
    Stack.MostRecentProperty = nullptr;

    // Get the pin attribute
    FOpenLogicPinHandle PinAttribute;
    Stack.Step(Stack.Object, &PinAttribute);

    Stack.Step(Stack.Object, NULL);
    FProperty* OutValueProp = CastField<FProperty>(Stack.MostRecentProperty);
    void* OutValuePtr = Stack.MostRecentPropertyAddress;

    P_FINISH;
    P_NATIVE_BEGIN;

    if (P_THIS->RuntimeGraph)
    {
        TSharedPtr<void> Value = P_THIS->GetRuntimeGraph()->GetDataPropertyValue(P_THIS, PinAttribute.PinName);
        if (Value.IsValid())
        {
            void* ValuePtr = Value.Get();

            OutValueProp->CopyCompleteValue(OutValuePtr, ValuePtr);
        }
    }
    
    P_NATIVE_END;
}

void UOpenLogicTask::Tick(float DeltaTime)
{
    if (IsRunning())
    {
        OnTaskTick(DeltaTime);
    }
}

TStatId UOpenLogicTask::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOpenLogicTask, STATGROUP_Tickables);
}

bool UOpenLogicTask::IsTickable() const
{
    if (!RuntimeGraph)
    {
        return false;
    }
    
    return IsRunning() && bIsTickable;
}

bool UOpenLogicTask::IsTickableInEditor() const
{
    return false;
}

FOpenLogicPinData UOpenLogicTask::GetInputPinData(int32 PinIndex) const
{
    return TaskData.InputPins[PinIndex];
}

FOpenLogicPinData UOpenLogicTask::GetOutputPinData(int32 PinIndex) const
{
    return TaskData.OutputPins[PinIndex];
}

TArray<FOpenLogicConnectablePin> UOpenLogicTask::GetConnectablePins(const FOpenLogicPinData& OtherPinInfo, EOpenLogicPinDirection OtherPinDirection) const
{
    TArray<FOpenLogicConnectablePin> ConnectablePins;

    const TArray<FOpenLogicPinData>& CandidatePins = OtherPinDirection == EOpenLogicPinDirection::Input
        ? TaskData.OutputPins
        : TaskData.InputPins;

    ConnectablePins.Reserve(CandidatePins.Num());

    UOpenLogicProperty* ThisProperty = nullptr;
    if (OtherPinInfo.Role == EPinRole::DataProperty)
    {
        ThisProperty = OtherPinInfo.PropertyClass.GetDefaultObject();
    }

    for (int32 Index = 0; Index < CandidatePins.Num(); ++Index)
    {
        const FOpenLogicPinData& CandidatePin = CandidatePins[Index];

        // Roles must match
        if (CandidatePin.Role != OtherPinInfo.Role)
        {
            continue;
        }

        // For data-property roles, check if the properties can connect
        if (CandidatePin.Role == EPinRole::DataProperty)
        {
            UOpenLogicProperty* OtherProperty = CandidatePin.PropertyClass.GetDefaultObject();
            
            if (!OtherProperty || !ThisProperty->CanConnectTo(OtherProperty))
            {
                continue;
            }
        }

        FOpenLogicConnectablePin ConnectablePin;
        ConnectablePin.PinIndex = Index;
        ConnectablePin.PinData = CandidatePin;
        ConnectablePins.Add(ConnectablePin);
    }

    return ConnectablePins;
}

int32 UOpenLogicTask::GetInputPinsCount() const
{
    if (!RuntimeGraph) return INDEX_NONE;

    TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = GetRuntimeGraph()->FindRuntimeNodeForTask(this);
    if (!RuntimeNode.IsValid())
    {
        return INDEX_NONE;
    }

    return RuntimeNode->InputPinsCount;
}

int32 UOpenLogicTask::GetOutputPinsCount() const
{
    if (!RuntimeGraph) return INDEX_NONE;
    
    TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = GetRuntimeGraph()->FindRuntimeNodeForTask(this);
    if (!RuntimeNode.IsValid())
    {
        return INDEX_NONE;
    }

    return RuntimeNode->OutputPinsCount;
}

bool UOpenLogicTask::IsRunning() const
{
    if (!RuntimeGraph) return false;

    TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode = GetRuntimeGraph()->FindRuntimeNodeForTask(this);
    if (!RuntimeNode.IsValid())
    {
        return false;
    }

    return RuntimeNode->TaskState == EOpenLogicTaskState::Running;
}

void UOpenLogicTask::SetGuid(FGuid Guid)
{
    NodeGuid = Guid;
}

FGuid UOpenLogicTask::GetGuid() const
{
    return NodeGuid;
}

void UOpenLogicTask::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
}

void UOpenLogicTask::OnTaskTick_Implementation(float DeltaTime)
{
}

void UOpenLogicTask::OnTaskCompleted_Implementation()
{
}

void UOpenLogicTask::OnGraphNodeInitialized_Implementation(UNodeBase* Node)
{
}

void UOpenLogicTask::OnGraphNodeSaved_Implementation(UNodeBase* Node)
{
}

void UOpenLogicTask::ExecutePin(int PinID)
{
    if (!GetRuntimeGraph() || !IsRunning())
    {
        return;
    }

    GetRuntimeGraph()->Then(this, PinID);
}

void UOpenLogicTask::ExecutePinByName(FName PinName)
{
    if (!IsRunning())
    {
        return;
    }
    
    FOpenLogicNode NodeData = GetRuntimeGraph()->GetNodeData(GetGuid());

    int32 OutputPinIndex = NodeData.GetOutputPinIndexFromName(PinName);
    if (OutputPinIndex == INDEX_NONE)
    {
        UE_LOG(OpenLogicLog, Error, TEXT("[ExecutePinByName] Invalid output pin index."));
        return;
    }

    ExecutePin(OutputPinIndex);
}

void UOpenLogicTask::ExecutePinByAttribute(FOpenLogicPinHandle PinName)
{
    ExecutePinByName(PinName.PinName);
}

/* Retrieves an input parameter based on its class and pin index */
UOpenLogicProperty* UOpenLogicTask::GetInputProperty(TSubclassOf<UOpenLogicProperty> ParamClass, int32 InputPin)
{
    return nullptr;
}

/* Retrieves an output parameter based on its class and pin index */
UOpenLogicProperty* UOpenLogicTask::GetOutputProperty(TSubclassOf<UOpenLogicProperty> ParamClass, int32 OutputPin)
{
   return GetOutputDynamicParameter(OutputPin);
}

UObject* UOpenLogicTask::GetContext() const
{
    UOpenLogicRuntimeGraph* Worker = GetRuntimeGraph();
    if (!Worker)
    {
        return nullptr;
    }

    return Worker->GetOuter();
}

void UOpenLogicTask::SetRuntimeGraph(UOpenLogicRuntimeGraph* Graph)
{
    RuntimeGraph = Graph;
}

UOpenLogicRuntimeGraph* UOpenLogicTask::GetRuntimeGraph() const
{
    return RuntimeGraph;
}

void UOpenLogicTask::SetExecutionHandleIndex(int32 Index)
{
    ExecutionHandleIndex = Index;
}

int32 UOpenLogicTask::GetExecutionHandleIndex() const
{
    return ExecutionHandleIndex;
}

TMap<int32, UOpenLogicProperty*>& UOpenLogicTask::GetDynamicProperties()
{
	return DynamicProperties;
}

UOpenLogicProperty* UOpenLogicTask::GetOutputDynamicParameter(int32 OutputPin)
{
    if (!DynamicProperties.Contains(OutputPin))
    {
		return nullptr;
	}

	return DynamicProperties[OutputPin];
}

void UOpenLogicTask::ResetTaskState()
{
    NodeGuid.Invalidate();
    RuntimeGraph = nullptr;
    ExecutionHandleIndex = INDEX_NONE;
    DynamicProperties.Empty();
}

#if WITH_EDITOR

bool UOpenLogicTask::CanEditChange(const FProperty* InProperty) const
{
    const bool bIsEditable = Super::CanEditChange(InProperty);

    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UOpenLogicTask, IsEventMultiImplementable))
    {
        return bIsEditable && TaskData.Type == ENodeType::Event;
    }

    return bIsEditable;
}

void UOpenLogicTask::PostCDOCompiled(const FPostCDOCompiledContext& Context)
{
    RefreshBlueprintTaskProperties();
    
    Super::PostCDOCompiled(Context);
}

void UOpenLogicTask::RefreshBlueprintTaskProperties()
{
    // Clear the current blueprint properties
    BlueprintTaskProperties.Empty();

    // Get the class we are currently processing
    UClass* TaskClass = this->GetClass();

    // Iterate through the class hierarchy to find all associated properties
    UBlueprint* Blueprint = Cast<UBlueprint>(TaskClass->ClassGeneratedBy);

    if (!Blueprint)
    {
        return;
    }
    
    for (auto VariableData : Blueprint->NewVariables)
    {
        FProperty* Property = this->GetClass()->FindPropertyByName(VariableData.VarName);
        if (!Property) continue;

        if (Property->IsA(FMulticastDelegateProperty::StaticClass()))
        {
            continue;
        }

        BlueprintTaskProperties.Add(VariableData.VarGuid, VariableData.VarName);
    }
}

#endif
