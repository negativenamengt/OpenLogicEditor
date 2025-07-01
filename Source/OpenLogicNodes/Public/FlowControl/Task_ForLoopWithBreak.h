// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_ForLoopWithBreak.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_ForLoopWithBreak : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_ForLoopWithBreak(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;

protected:
	UPROPERTY()
		bool bBreak = false;
};