// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_Delay.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_Delay : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_Delay(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;

protected:
	UFUNCTION()
		void OnDelayCompleted();
};