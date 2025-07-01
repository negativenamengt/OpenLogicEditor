// Fill out your copyright notice in the Description page of Project Settings.


#include "Runtime/OpenLogicRuntimeGraphComponent.h"

#include "Utility/OpenLogicUtility.h"

// Sets default values for this component's properties
UOpenLogicRuntimeGraphComponent::UOpenLogicRuntimeGraphComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UOpenLogicRuntimeGraphComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create the runtime graph
	RuntimeGraph = UOpenLogicUtility::CreateRuntimeGraphFromObject(this, this, GraphAsset);

	// Bind the events
	if (RuntimeGraph)
	{
		RuntimeGraph->OnNodeActivated.AddDynamic(this, &UOpenLogicRuntimeGraphComponent::HandleNodeActivated);
		RuntimeGraph->OnNodeCompleted.AddDynamic(this, &UOpenLogicRuntimeGraphComponent::HandleNodeCompleted);
	}
	
	if (RuntimeGraph && BeginPlayEvent)
	{
		RuntimeGraph->TriggerAllEvents(BeginPlayEvent);
	}
}

void UOpenLogicRuntimeGraphComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (RuntimeGraph && EndPlayEvent)
	{
		RuntimeGraph->TriggerAllEvents(EndPlayEvent);
	}
}

// Called every frame
void UOpenLogicRuntimeGraphComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UOpenLogicRuntimeGraphComponent::HandleNodeActivated(UOpenLogicTask* NewActivatedNode)
{
	// Trigger the component's OnNodeActivated delegate
	OnNodeActivated.Broadcast(NewActivatedNode);
}

void UOpenLogicRuntimeGraphComponent::HandleNodeCompleted(UOpenLogicTask* NewCompletedNode)
{
	// Trigger the component's OnNodeCompleted delegate
	OnNodeCompleted.Broadcast(NewCompletedNode);
}

