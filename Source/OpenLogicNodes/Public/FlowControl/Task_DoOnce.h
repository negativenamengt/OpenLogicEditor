// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_DoOnce.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_DoOnce : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_DoOnce(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;

protected:
	UPROPERTY()
		bool bFirstEntrance = true;

	UPROPERTY()
		bool bIsClosed = false;
};