// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DisplayableWidgetBase.generated.h"

class UNodeBase;

UCLASS(Abstract)
class OPENLOGICV2_API UDisplayableWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UNodeBase* GetOpenLogicNode() const { return Node; }

public:
	UPROPERTY()
		UNodeBase* Node;
};
