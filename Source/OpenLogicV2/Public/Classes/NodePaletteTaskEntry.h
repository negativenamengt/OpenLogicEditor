// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "NodePaletteBaseItem.h"
#include "Tasks/OpenLogicTask.h"
#include "NodePaletteTaskEntry.generated.h"

UCLASS()
class OPENLOGICV2_API UNodePaletteTaskEntry : public UNodePaletteBaseItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic")
		TSubclassOf<UOpenLogicTask> TaskClass;
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic")
		FString Metadata;
};
