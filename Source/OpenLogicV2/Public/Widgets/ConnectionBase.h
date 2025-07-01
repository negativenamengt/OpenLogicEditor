// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConnectionBase.generated.h"

class UExecutionPinBase;
class UGraphEditorBase;

UCLASS(Deprecated)
class OPENLOGICV2_API UDEPRECATED_ConnectionBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic|Event")
		void RecalculateConnection();

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets")
		UGraphEditorBase* GraphEditor;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		UExecutionPinBase* GetSourcePin() const { return StartPin; }

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		UExecutionPinBase* GetTargetPin() const { return EndPin; }

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Pin", meta = (DeprecatedProperty, DeprecationMessage = "Direct access to StartPin is deprecated. Please use the getter."))
		UExecutionPinBase* StartPin;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Pin", meta = (DeprecatedProperty, DeprecationMessage = "Direct access to EndPin is deprecated. Please use the getter."))
		UExecutionPinBase* EndPin;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		bool GetIsPreviewConnection() const { return IsPreviewConnection; }

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Data", meta = (ExposeOnSpawn = "true"))
		bool IsPreviewConnection;

};
