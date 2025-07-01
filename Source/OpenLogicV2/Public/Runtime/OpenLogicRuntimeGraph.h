// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "OpenLogicRuntimeGraph.generated.h"

// Forward declarations
class UOpenLogicRuntimeEventContext;
class FOpenLogicGraphRunnable;
class UOpenLogicTask;

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRuntimeWorkerNodeActivated, UOpenLogicTask*, NewActivatedNode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRuntimeWorkerNodeCompleted, UOpenLogicTask*, NewCompletedNode);

UCLASS(Blueprintable)
class OPENLOGICV2_API UOpenLogicRuntimeGraph : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Executes the event for the specified task class.
	 * @param TaskClass	The class of the task to trigger.
	 * @param bSuccess True if the event was triggered successfully, false otherwise.
	 */
	UE_DEPRECATED(5.0, "Use TriggerEvent instead.")
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Event", meta = (DeprecatedFunction, DeprecationMessage = "Use TriggerEvent instead."))
	void MultiExecuteEvent(TSubclassOf<UOpenLogicTask> TaskClass, bool& bSuccess);

public:
	/**
	 * Sets the graph data for the runtime graph.
	 * @param NewData The new graph data to set.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Graph")
	void SetGraphData(FOpenLogicGraphData NewData);

	/**
	 * Returns the graph data for the runtime graph.
	 * @return The graph data.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Graph")
	FOpenLogicGraphData GetGraphData() const { return WorkerGraphData; }

	/**
	 * Returns the node data for the specified node
	 * @param NodeID The ID of the node to retrieve data for.
	 * @return The node data for the specified ID.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Graph")
	FOpenLogicNode GetNodeData(FGuid NodeID) const;

	/**
	 * Sets the thread type of the runtime graph. This should be called before the worker is created.
	 * @param NewThreadSettings	The new thread settings to set.
	 */
	UFUNCTION()
	void SetThreadSettings(FOpenLogicThreadSettings& NewThreadSettings);

	/**
	 * Returns the thread settings of the runtime graph.
	 * @return The thread settings of the runtime graph.
	 */
	UFUNCTION(BlueprintPure, Category = OpenLogic)
	FOpenLogicThreadSettings GetThreadSettings() const { return ThreadSettings; }

	/**
	 * Sets the context object for the runtime graph. This context is passed to the task instances.
	 * @param NewContextObject The new context object to set.
	 */
	UFUNCTION()
	void SetContext(UObject* NewContextObject);

	/**
	 * Returns the context object of the runtime graph.
	 * @return The context object of the runtime graph.
	 */
	UFUNCTION(BlueprintPure, Category = OpenLogic)
	UObject* GetContext() const { return ContextObject; }

	/**
	 * Adds the specified execution handle to the queue for processing.
	 * @param ExecutionHandle The execution handle to process.
	 * @return True if the execution handle was processed successfully, false otherwise.
	 */
	bool AddExecutionHandleToQueue(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);

	/**
	 * Destroys the runtime worker and cleans up resources.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Graph")
	void DestroyWorker();

	/**
	 * Cleans up the runnable thread if it exists.
	 */
	UFUNCTION()
	void CleanupThread();
	
	/**
	 * This function triggers the first found event implementation of the specified Task class
	 * @param TaskClass The class of the task to trigger.
	 * @param AutoProcess If true, the event will be processed automatically.
	 * @param OutExecutionHandle The execution handle for the triggered event.
	 * @return True if the event was triggered successfully, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		bool TriggerEvent(TSubclassOf<UOpenLogicTask> TaskClass, bool AutoProcess, FOpenLogicGraphExecutionHandle& OutExecutionHandle);

	/**
	 * This function triggers all event implementations of the specified Task class
	 * @param TaskClass The class of the task to trigger.
	 * @param AutoProcess If true, the events will be processed automatically.
	 * @return An array of execution handles for the triggered events.
	 */
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		TArray<FOpenLogicGraphExecutionHandle> TriggerAllEvents(TSubclassOf<UOpenLogicTask> TaskClass, bool AutoProcess = true);

	/**
	 * Creates a new execution handle for the specified node ID.
	 * @param NodeID The ID of the node to create an execution handle for.
	 * @return A shared pointer to the created execution handle.
	 */
	TSharedPtr<FOpenLogicGraphExecutionHandle> CreateExecutionHandle(FGuid NodeID);

	/**
	 * Retrieves the execution handle at the specified index.
	 * @param HandleIndex The index of the execution handle to retrieve.
	 * @return A shared pointer to the execution handle at the specified index.
	 */
	TSharedPtr<FOpenLogicGraphExecutionHandle> GetExecutionHandle(int32 HandleIndex) const;

	/**
	 * Destroys the specified execution handle. This function will clean up all resources associated with the handle (runtime nodes, task instances, etc.).
	 * @param ExecutionHandle The execution handle to destroy.
	 */
	void DestroyExecutionHandle(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);

	UFUNCTION(BlueprintCallable, Category = OpenLogic, meta = (DisplayName = "Destroy Execution Handle"))
	void BP_DestroyExecutionHandle(FOpenLogicGraphExecutionHandle ExecutionHandle);
	
	/**
	 * Retrieves all execution handles.
	 * @return An array of all execution handles.
	 */
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		TArray<FOpenLogicGraphExecutionHandle> GetExecutionHandles() const;

	/**
	 * Checks if the specified execution handle is valid.
	 * @param ExecutionHandle The execution handle to check.
	 * @return True if the execution handle is valid, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		static bool IsExecutionHandleValid(const FOpenLogicGraphExecutionHandle& ExecutionHandle);

	/**
	 * Finds the execution handle for the specified task instance.
	 * @param TaskInstance The task instance to find the execution handle for.
	 * @return A shared pointer to the execution handle for the specified task instance.
	 */
	TSharedPtr<FOpenLogicGraphExecutionHandle> FindExecutionHandleForTask(const UOpenLogicTask* TaskInstance) const;

	/**
	 * Finds the runtime node for the specified task instance.
	 * @param TaskInstance The task instance to find the runtime node for.
	 * @return A shared pointer to the runtime node for the specified task instance.
	 */
	TSharedPtr<FOpenLogicRuntimeNode> FindRuntimeNodeForTask(const UOpenLogicTask* TaskInstance) const;

	/**
	 * Activates the specified node with the given pin name.
	 * @param RuntimeNode The runtime node to activate.
	 * @param PinName The name of the pin to activate.
	 */
	void ActivateNode(TSharedPtr<FOpenLogicRuntimeNode> RuntimeNode, const FName& PinName);

	/**
	 * Completes the specified node and performs necessary cleanup.
	 * @param TaskInstance The task instance to complete.
	 */
	void CompleteNode(UOpenLogicTask* TaskInstance);

	/**
	 * Transitions to the next node in the sequence.
	 * @param TaskInstance The task instance to transition from.
	 * @param NextPinIndex The index of the next pin to transition to.
	 * @return True if the transition was successful, false otherwise.
	 */
	bool Then(UOpenLogicTask* TaskInstance, int32 NextPinIndex = 1);

	/**
	 * Sets the value of the specified data property.
	 * @param TaskInstance The task instance to set the property for.
	 * @param PinName The name of the pin to set the property for.
	 * @param Value The value to set.
	 */
	void SetDataPropertyValue(UOpenLogicTask* TaskInstance, FName PinName, const TSharedPtr<void>& Value) const;

	void SetDataPropertyValueByAddress(UOpenLogicTask* TaskInstance, FName PinName, FProperty* Property, void* SourceAddress) const;

	/**
	 * Retrieves the value of the specified data property.
	 * @param TaskInstance The task instance to retrieve the property for.
	 * @param PinName The name of the pin to retrieve the property for.
	 * @return A shared pointer to the value of the property.
	 */
	TSharedPtr<void> GetDataPropertyValue(UOpenLogicTask* TaskInstance, FName PinName);

	void PreloadInputPropertiesForNode(const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);
	TSharedPtr<void> ResolveConnectedPinValue(const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle, const FOpenLogicPinConnection& Connection);
	TSharedPtr<void> CreatePropertyValueFromDefault(const FOpenLogicDefaultValue& DefaultValue, const UOpenLogicProperty* PropertyInstance);
	
	/**
	 * Retrieves the default value of the specified data property.
	 * @param TaskInstance The task instance to retrieve the default value for.
	 * @param PinName The name of the pin to retrieve the default value for.
	 * @return The default value of the property.
	 */
	FOpenLogicDefaultValue GetDefaultValue(UOpenLogicTask* TaskInstance, FName PinName);

	/**
	 * Retrieves the number of execution handles created so far.
	 * @return The count of execution handles.
	 */
	int32 GetHandleCount() const { return HandleCounter; }

	/**
	 * Increments the handle counter and returns the next handle index.
	 * @return The next handle index.
	 */
	int32 GetNextHandleIndex() { return HandleCounter++; }

	/**
	 * Processes the specified node by its GUID.
	 * @param NodeID The ID of the node to process.
	 * @param ExecutionHandle The execution handle to use for processing.
	 * @return A shared pointer to the processed runtime node.
	 */
	TSharedPtr<FOpenLogicRuntimeNode> ProcessNodeByGUID(FGuid NodeID, TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle);

	/**
	 * Dispatcher triggered when a node is activated.
	 */
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic")
	FRuntimeWorkerNodeActivated OnNodeActivated;

	/**
	 * Dispatcher triggered when a node is completed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic")
	FRuntimeWorkerNodeCompleted OnNodeCompleted;

protected:
	UPROPERTY()
	TMap<TSubclassOf<UOpenLogicTask>, FOpenLogicTaskPool> TaskPools;

	UPROPERTY()
	TMap<FGuid, UOpenLogicTask*> PersistentNodes;

private:
	/**
	 * Retrieves the event implementations for the specified task class.
	 * @param TaskClass The class of the task to retrieve event implementations for.
	 * @return An array of GUIDs representing the event implementations for the specified task class.
	 */
	UFUNCTION()
	TArray<FGuid> GetEventImplementations(TSubclassOf<UOpenLogicTask> TaskClass) const;

	/**
	 * Creates or retrieves a runtime node for the specified ID.
	 * @param NodeID The ID of the node to retrieve or create.
	 * @param ExecutionHandle The execution handle to use for processing.
	 * @return A shared pointer to the runtime node for the specified ID.
	 */
	TSharedPtr<FOpenLogicRuntimeNode> GetOrCreateRuntimeNode(FGuid NodeID, TSharedPtr<FOpenLogicGraphExecutionHandle> ExecutionHandle);

	/**
	 * Creates or retrieves a task instance for the specified runtime node and execution handle.
	 * @param RuntimeNode The runtime node to create or retrieve the task instance for.
	 * @param ExecutionHandle The execution handle to use for processing.
	 * @return A pointer to the task instance for the specified runtime node and execution handle.
	 */
	UOpenLogicTask* GetOrCreateTaskInstance(TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);

	/**
	 * Initializes the task instance with the specified runtime node and execution handle.
	 * @param TaskInstance The task instance to initialize.
	 * @param RuntimeNode The runtime node to initialize the task instance for.
	 * @param ExecutionHandle The execution handle to use for processing.
	 */
	void InitializeTaskInstance(UOpenLogicTask* TaskInstance, const TSharedPtr<FOpenLogicRuntimeNode>& RuntimeNode, const TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);

	/**
	 * Processes the specified execution handle.
	 * @param ExecutionHandle The execution handle to process.
	 */
	void ProcessExecutionHandle(TSharedPtr<FOpenLogicGraphExecutionHandle>& ExecutionHandle);

	UPROPERTY()
	FOpenLogicGraphData WorkerGraphData;

	UPROPERTY()
	int32 HandleCounter = 0;
	
	TMap<int32, TSharedPtr<FOpenLogicGraphExecutionHandle>> HandleRegistry;

	UPROPERTY()
	FOpenLogicThreadSettings ThreadSettings;

	UPROPERTY()
	UObject* ContextObject = nullptr;

	// The runnable that contains the execution logic for the runtime graph
	FOpenLogicGraphRunnable* Runnable = nullptr;

	// The thread that will execute the runnable in the background
	FRunnableThread* RunnableThread = nullptr;
};