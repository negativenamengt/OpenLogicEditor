// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "OpenLogicProperty.h"
#include "Tickable.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "OpenLogicTask.generated.h"

class UNodeBase;
class UDisplayableWidgetBase;
class UOpenLogicRuntimeEventContext;

UCLASS(Blueprintable, BlueprintType, Meta = (ShowWorldContextPin), Abstract)
class OPENLOGICV2_API UOpenLogicTask : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta=(ShowOnlyInnerProperties))
		FTaskData TaskData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Node Settings")
		bool IsEventMultiImplementable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph Editor")
		TArray<TSubclassOf<UDisplayableWidgetBase>> DisplayableWidgets;

	// Shows the payload editor in the node widget.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph Editor")
		bool ShowPayloadEditor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graph Editor")
		bool ShowInNodePalette = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
		ENodeLifecycle NodeLifecycle = ENodeLifecycle::Custom;

	// If true, the node will be reevaluated each time it is processed.
	// It is recommended to use this feature only when necessary, as it may affect performance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Runtime")
		bool ReevaluateOnDemand = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Runtime")
		bool bIsTickable = false;

public:
	UFUNCTION()
		FOpenLogicPinData GetInputPinData(int32 PinIndex) const;

	UFUNCTION()
		FOpenLogicPinData GetOutputPinData(int32 PinIndex) const;

	// Returns all pins on this task that can connect to OtherPinInfo.
	UFUNCTION()
		TArray<FOpenLogicConnectablePin> GetConnectablePins(const FOpenLogicPinData& OtherPinInfo, EOpenLogicPinDirection OtherPinDirection) const;

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		int32 GetInputPinsCount() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		int32 GetOutputPinsCount() const;

public:
	// Returns whether the task is currently running in runtime.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool IsRunning() const;

public:
	// Updates the runtime unique identifier of the task.
	UFUNCTION()
		void SetGuid(FGuid Guid);

	// Returns the runtime unique identifier of the task.
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Runtime")
		FGuid GetGuid() const;
	
private:
	// The unique identifier for the task node.
	UPROPERTY()
		FGuid NodeGuid;

public:
	// Called when the runtime instance starts.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnTaskActivated(UObject* Context, FName PinName = NAME_None);

	// Called when the runtime instance ticks.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnTaskTick(float DeltaTime);

	// Called when the runtime instance stops.
	// This is where you should perform cleanup (unbinding delegates, etc.).
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnTaskCompleted();

public:
	// Event triggered when a node widget that uses this task class is initialized.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnGraphNodeInitialized(UNodeBase* Node);

	// Event triggered when a node widget that uses this task class is saved.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnGraphNodeSaved(UNodeBase* Node);

public:
	// Triggers a specific output pin to execute sequences of tasks.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void ExecutePin(int PinID = 1);

	// Triggers a specific output pin by name to execute sequences of tasks.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void ExecutePinByName(FName PinName);

	// Triggers a specific output pin by attribute to execute sequences of tasks.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (PinDirection = "Output", PinRole = "FlowControl"))
		void ExecutePinByAttribute(FOpenLogicPinHandle PinName);
	
	// Marks this task as completed.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (DisplayName = "Complete Task", PinDirection = "Output", PinRole = "FlowControl"))
		void CompleteTask(FOpenLogicPinHandle PinName = FOpenLogicPinHandle());

	// Marks this task as completed and executes the specified output pin.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void ExecuteAndCompleteTask(int32 PinIndex = 1);

public:
	/* Completes a task and performs necessary cleanup. */
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = ( DeprecatedFunction, DeprecationMessage= "Please use the new 'CompleteTask' and 'CallOutputPin' functions to ensure compatibility with the runtime worker."))
		void ReturnTask(int ReturnPin = 1);

public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic, CustomThunk, meta = (CustomStructureParam = "Value", PinDirection = "Output", PinRole = "DataProperty"), DisplayName = "Set Property Value By Attribute")
		void BP_SetPropertyValueByAttribute(FOpenLogicPinHandle PinName, TFieldPath<FProperty> Value);
		DECLARE_FUNCTION(execBP_SetPropertyValueByAttribute);
	
	UFUNCTION(BlueprintPure, Category = OpenLogic, CustomThunk, meta = (CustomStructureParam = "OutValue", PinRole = "DataProperty"), DisplayName = "Get Property Value By Attribute")
		void BP_GetPropertyValueByAttribute(FOpenLogicPinHandle PinName, TFieldPath<FProperty>& OutValue);
		DECLARE_FUNCTION(execBP_GetPropertyValueByAttribute);
	
	template <typename T>
	void SetPropertyValueByAttribute(FName PinName, const T& Value)
	{
		if (!RuntimeGraph)
        {
            return;
        }

		RuntimeGraph->SetDataPropertyValue(this, PinName, MakeShared<T>(Value));
	}

	template <typename T>
	T GetPropertyValueByAttribute(FName PinName)
	{
		if (!GetRuntimeGraph())
		{
			return T();
		}

		TSharedPtr<void> Value = GetRuntimeGraph()->GetDataPropertyValue(this, PinName);
		if (!Value.IsValid())
		{
			FOpenLogicDefaultValue DefaultValue = RuntimeGraph->GetDefaultValue(this, PinName);
			if (DefaultValue.IsEmpty())
			{
				return T();
			}

			T OutValue;
			if (!DefaultValue.GetValue<T>(OutValue))
			{
				return T();
			}

			return OutValue;
		}

		return *StaticCastSharedPtr<T>(Value);
	}

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Property", CustomThunk, meta = (CustomStructureParam = "Value", OutputPinIndex = "1"))
		void SetOutputPropertyValue(int32 OutputPinIndex, TFieldPath<FProperty> Value);
		DECLARE_FUNCTION(execSetOutputPropertyValue)
	{
		Stack.MostRecentProperty = nullptr;

		// Fetch the InputPinIndex of type int32
		int32 OutputPinIndex;
		Stack.StepCompiledIn<FIntProperty>(&OutputPinIndex);

		// Fetch the TFieldPath<FProperty> Value
		Stack.Step(Stack.Object, NULL);
		FProperty* ValueProp = CastField<FProperty>(Stack.MostRecentProperty);
		void* ValuePtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		P_NATIVE_BEGIN;

		// Using the GetInputParam function to get the UOpenLogicProperty instance
		UOpenLogicProperty* ParamInstance = P_THIS->GetOutputProperty(nullptr, OutputPinIndex);

		if (ParamInstance)
		{
			FProperty* TargetProperty = FindFProperty<FProperty>(ParamInstance->GetClass(), TEXT("Value"));

			if (TargetProperty)
			{
				void* TargetAddress = TargetProperty->ContainerPtrToValuePtr<void>(ParamInstance);
				TargetProperty->CopySingleValue(TargetAddress, ValuePtr);
			}
		}

		P_NATIVE_END;
	}

	UFUNCTION(BlueprintPure, Category = "OpenLogic|Property", CustomThunk, meta = (CustomStructureParam = "OutValue", InputPinIndex = "1"))
		void GetInputPropertyValue(int32 InputPinIndex, TFieldPath<FProperty>& OutValue);
		DECLARE_FUNCTION(execGetInputPropertyValue)
		{
			Stack.MostRecentProperty = nullptr;

			// Fetch the InputPinIndex of type int32
			int32 InputPinIndex;
			Stack.StepCompiledIn<FIntProperty>(&InputPinIndex);

			// Here we will prepare the OutValue. Instead of getting a value, we're going to set one.
			Stack.Step(Stack.Object, NULL);
			FProperty* OutValueProp = CastField<FProperty>(Stack.MostRecentProperty);
			void* OutValuePtr = Stack.MostRecentPropertyAddress;

			P_FINISH;
			P_NATIVE_BEGIN;

			UOpenLogicProperty* ParamInstance = P_THIS->GetInputProperty(nullptr, InputPinIndex);
			if (ParamInstance)
			{
				FProperty* SourceProperty = FindFProperty<FProperty>(ParamInstance->GetClass(), TEXT("Value"));
				if (SourceProperty)
				{
					void* SourceAddress = SourceProperty->ContainerPtrToValuePtr<void>(ParamInstance);
					OutValueProp->CopySingleValue(OutValuePtr, SourceAddress);
				}
			}

			P_NATIVE_END;
		}

	UFUNCTION(BlueprintPure, Category = "OpenLogic|Property", CustomThunk, meta = (CustomStructureParam = "OutValue", OutputPinIndex = "1"))
		void GetOutputPropertyValue(int32 OutputPinIndex, TFieldPath<FProperty>& OutValue);
		DECLARE_FUNCTION(execGetOutputPropertyValue)
		{
			Stack.MostRecentProperty = nullptr;

			// Fetch the OutputPinIndex of type int32
			int32 OutputPinIndex;
			Stack.StepCompiledIn<FIntProperty>(&OutputPinIndex);

			// Here we will prepare the OutValue. Instead of getting a value, we're going to set one.
			Stack.Step(Stack.Object, NULL);
			FProperty* OutValueProp = CastField<FProperty>(Stack.MostRecentProperty);
			void* OutValuePtr = Stack.MostRecentPropertyAddress;

			P_FINISH;
			P_NATIVE_BEGIN;

			UOpenLogicProperty* ParamInstance = P_THIS->GetOutputProperty(nullptr, OutputPinIndex);
			if (ParamInstance)
			{
				FProperty* SourceProperty = FindFProperty<FProperty>(ParamInstance->GetClass(), TEXT("Value"));
				if (SourceProperty)
				{
					void* SourceAddress = SourceProperty->ContainerPtrToValuePtr<void>(ParamInstance);
					OutValueProp->CopySingleValue(OutValuePtr, SourceAddress);
				}
			}

			P_NATIVE_END;
		}

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Property", meta = (DeterminesOutputType = "ParamClass"))
		UOpenLogicProperty* GetInputProperty(TSubclassOf<UOpenLogicProperty> ParamClass, int32 InputPin = 1);

	UFUNCTION(BlueprintPure, Category = "OpenLogic|Property", meta = (DeterminesOutputType = "ParamClass"))
		UOpenLogicProperty* GetOutputProperty(TSubclassOf<UOpenLogicProperty> ParamClass, int32 OutputPin = 1);

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UObject* GetContext() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsRunningInGameThread() const { return IsInGameThread(); }

public:
	// Sets the runtime graph that the task node belongs to.
	UFUNCTION()
		void SetRuntimeGraph(UOpenLogicRuntimeGraph* Graph);
	
	// Returns the runtime graph of the runtime context.
	UFUNCTION()
	    UOpenLogicRuntimeGraph* GetRuntimeGraph() const;

	UFUNCTION()
		void SetExecutionHandleIndex(int32 Index);

	UFUNCTION()
		int32 GetExecutionHandleIndex() const;

	// Returns the runtime output parameters of the task.
	UFUNCTION()
		TMap<int32, UOpenLogicProperty*>& GetDynamicProperties();

	UFUNCTION()
		UOpenLogicProperty* GetOutputDynamicParameter(int32 OutputPin);

	UFUNCTION()
		void ResetTaskState();
private:
	UPROPERTY()
		TMap<int32, UOpenLogicProperty*> DynamicProperties;

	UPROPERTY()
		UOpenLogicRuntimeGraph* RuntimeGraph = nullptr;
	
	UPROPERTY()
		int32 ExecutionHandleIndex = INDEX_NONE;

public:
	// Returns all the blueprint properties of the task.
	UFUNCTION()
		TMap<FGuid, FName> GetBlueprintTaskProperties(bool bIncludeSuper = false) const;

	// Returns all the C++ properties of the task.
	UFUNCTION()
		TArray<FName> GetCPPTaskProperties() const;
	
	UFUNCTION()
		const FName GetPropertyNameByGUID(FGuid PropertyGUID);

	UFUNCTION()
		const FGuid GetPropertyIdentifierByName(FName PropertyName, bool bIncludeSuper = false);

	UFUNCTION()
		bool DoesPropertyIdentifierExist(FGuid PropertyGUID);

	UFUNCTION()
		bool IsPropertyStored(FName PropertyName);

	UFUNCTION()
		bool IsCppProperty(FName PropertyName) const;

public:
	UFUNCTION()
		TMap<FGuid, FName> GetDeprecatedTaskProperties();

protected:
	// Old map to store the variables of the task with their unique identifier.
	// This map is deprecated and should not be used. Use BlueprintTaskProperties instead.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Properties")
		TMap<FGuid, FName> TaskProperties;

	// Internal map to store the variables of the task with their unique identifier.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task Properties")
		TMap<FGuid, FName> BlueprintTaskProperties;

public:
	virtual UWorld* GetWorld() const override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack) override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

	#if WITH_EDITOR
		virtual bool CanEditChange(const FProperty* InProperty) const override;
		virtual void PostCDOCompiled(const FPostCDOCompiledContext& Context) override;

		void RefreshBlueprintTaskProperties();

	#endif
};
