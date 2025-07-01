// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Classes/OpenLogicGraph.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "OpenLogicRuntimeSubsystem.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicRuntimeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic")
		TArray<UOpenLogicRuntimeGraph*> ActiveRuntimeWorkers;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (DefaultToSelf = "Outer", DeprecatedFunction, DeprecationMessage = "Use CreateRuntimeWorkerFromObject or CreateRuntimeWorkerFromStruct static functions instead."))
		UOpenLogicRuntimeGraph* CreateRuntimeGraph(FOpenLogicGraphData GraphData, UObject* Outer);

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FOpenLogicGraphData GetGraphData(UOpenLogicGraph* Graph);
};