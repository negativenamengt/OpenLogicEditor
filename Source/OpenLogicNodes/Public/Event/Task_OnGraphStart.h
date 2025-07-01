// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_OnGraphStart.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_OnGraphStart : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_OnGraphStart(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;
};
