// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DisplayableWidgetPanel.generated.h"

class UDisplayableWidgetBase;

UCLASS(Deprecated)
class OPENLOGICV2_API UDEPRECATED_DisplayableWidgetPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic|Events")
		void OnWidgetDisplay(UDisplayableWidgetBase* WidgetToDisplay);
};
