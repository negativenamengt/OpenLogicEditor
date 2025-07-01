// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_DoN.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_DoN : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_DoN(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;

protected:
	UPROPERTY()
		int32 Counter = 0;
};