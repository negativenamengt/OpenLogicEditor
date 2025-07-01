// Copyright 2025 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_MakeLiteralDouble.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_MakeLiteralDouble : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_MakeLiteralDouble(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;
};
