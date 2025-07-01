// Copyright 2025 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Tasks/OpenLogicTask.h"
#include "Task_MakeLiteralString.generated.h"

UCLASS()
class OPENLOGICNODES_API UTask_MakeLiteralString : public UOpenLogicTask
{
	GENERATED_BODY()
public:
	UTask_MakeLiteralString(const FObjectInitializer& ObjectInitializer);

	virtual void OnTaskActivated_Implementation(UObject* Context, FName PinName) override;
};