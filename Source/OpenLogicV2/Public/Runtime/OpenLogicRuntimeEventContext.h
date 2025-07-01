// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "OpenLogicRuntimeEventContext.generated.h"

class UOpenLogicRuntimeGraph;
class FOpenLogicContextTicker;

UCLASS(Blueprintable, BlueprintType)
class OPENLOGICV2_API UOpenLogicRuntimeEventContext : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void ProcessEventNode();

	UFUNCTION(BlueprintPure, Category = OpenLogic)
        bool IsProcessed() const;

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool IsRunning() const;
	
	// Returns the initial event if it exists.
	// Useful to give data to the event task before executing it.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UOpenLogicTask* TryGetInitialEvent();

	// Returns the worker that is executing the event
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UOpenLogicRuntimeGraph* GetWorker() const;

	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void Shutdown();

public:
	// The class of the task
	UPROPERTY()
		TSubclassOf<UOpenLogicTask> TaskClass;

	// The unique node identifier for the event
	UPROPERTY()
		FGuid EventID;

	// The worker that is executing the event
	UPROPERTY()
		UOpenLogicRuntimeGraph* Worker;

public:
	UFUNCTION()
		void ProcessNodeByGuid(FGuid NodeGuid);

	UFUNCTION()
		bool Then(FOpenLogicRuntimeNode& SourceNode, int32 OutputPinIndex);

public:
	UFUNCTION()
		FOpenLogicRuntimeNode GetRuntimeNode(FGuid NodeGuid) const;

	UFUNCTION()
		bool IsNodeProcessed(FGuid NodeGuid) const;
	
private:
	UFUNCTION()
		void InitializeRuntimeNode(FGuid NodeGuid);
	
	UFUNCTION()
		FOpenLogicRuntimeNode GetOrCreateRuntimeNode(FGuid NodeGuid);

	UFUNCTION()
		FOpenLogicNode GetGraphNodeData(FGuid NodeGuid) const;

private:
	// A map that stores the runtime nodes during execution.
	// This is used to keep track of the nodes that are currently being executed.
	UPROPERTY()
		TMap<FGuid, FOpenLogicRuntimeNode> RuntimeNodes;

	// Returns whether the event context has been processed
	UPROPERTY()
		bool bProcessed = false;

	// Returns whether the event context is running
	UPROPERTY()
		bool bRunning = false;
};
