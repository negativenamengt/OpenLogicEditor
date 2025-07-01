// Copyright 2025 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_MakeLiteralFloat.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_MakeLiteralFloat : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_MakeLiteralFloat(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;
};
