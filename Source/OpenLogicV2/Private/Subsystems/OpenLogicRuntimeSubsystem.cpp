// Copyright 2024 - NegativeNameSeller

#include "Subsystems/OpenLogicRuntimeSubsystem.h"

/* Creates and initializes a graph runtime worker */
UOpenLogicRuntimeGraph* UOpenLogicRuntimeSubsystem::CreateRuntimeGraph(FOpenLogicGraphData GraphData, UObject* Outer)
{
	UOpenLogicRuntimeGraph* NewRuntimeWorker = NewObject<UOpenLogicRuntimeGraph>(Outer, UOpenLogicRuntimeGraph::StaticClass());
	NewRuntimeWorker->SetGraphData(GraphData);

	ActiveRuntimeWorkers.Add(NewRuntimeWorker);
	return NewRuntimeWorker;
}

FOpenLogicGraphData UOpenLogicRuntimeSubsystem::GetGraphData(UOpenLogicGraph* Graph)
{
	if (IsValid(Graph))
	{
		return Graph->GraphData;
	}
	return FOpenLogicGraphData();
}