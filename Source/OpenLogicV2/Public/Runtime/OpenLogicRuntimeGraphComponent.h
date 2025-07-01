// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "OpenLogicRuntimeGraph.h"
#include "Classes/OpenLogicGraph.h"
#include "Components/ActorComponent.h"
#include "OpenLogicRuntimeGraphComponent.generated.h"

UCLASS( ClassGroup=(OpenLogic), meta=(BlueprintSpawnableComponent) )
class OPENLOGICV2_API UOpenLogicRuntimeGraphComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOpenLogicRuntimeGraphComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the component is being destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic, meta = (ExposeOnSpawn = true))
		UOpenLogicGraph* GraphAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenLogic|Default Events", meta = (ExposeOnSpawn = true))
		TSubclassOf<UOpenLogicTask> BeginPlayEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenLogic|Default Events", meta = (ExposeOnSpawn = true))
		TSubclassOf<UOpenLogicTask> EndPlayEvent;

public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UOpenLogicRuntimeGraph* GetRuntimeGraph() const { return RuntimeGraph; }

public:
	// Triggered when a node is activated
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic")
		FRuntimeWorkerNodeActivated OnNodeActivated;

	// Triggered when a node is completed
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic")
		FRuntimeWorkerNodeCompleted OnNodeCompleted;

private:
	UFUNCTION()
		void HandleNodeActivated(UOpenLogicTask* NewActivatedNode);

	UFUNCTION()
		void HandleNodeCompleted(UOpenLogicTask* NewCompletedNode);
	
private:
	UPROPERTY()
		UOpenLogicRuntimeGraph* RuntimeGraph = nullptr;
};
