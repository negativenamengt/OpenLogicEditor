// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenLogicPayloadWidget.generated.h"

class UOpenLogicTask;
class UNodeBase;

UCLASS(Deprecated)
class OPENLOGICV2_API UDEPRECATED_OpenLogicPayloadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Event called when the payload is initialized
	UFUNCTION(BlueprintNativeEvent)
		void OnPayloadInitialized();

public:
	// The task object that this payload is inspecting.
	UPROPERTY(BlueprintReadWrite, Category = "OpenLogic")
		UOpenLogicTask* TaskObject;

	// The node widget that owns this payload.
	UPROPERTY(BlueprintReadWrite, Category = "OpenLogic")
		UNodeBase* NodeWidget;

private:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void AlertPropertyChange(FName PropertyName);
};
