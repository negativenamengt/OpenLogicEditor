// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OpenLogicTaskCollection.generated.h"

class UOpenLogicTask;

UCLASS(BlueprintType)
class OPENLOGICV2_API UOpenLogicTaskCollection : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenLogic")
		TArray<TSoftClassPtr<UOpenLogicTask>> CollectionTasks;
};
