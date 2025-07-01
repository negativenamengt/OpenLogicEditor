// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Blueprint/UserWidget.h"
#include "Core/OpenLogicTypes.h"
#include "CustomConnection.generated.h"

class UExecutionPinBase;
class UGraphEditorBase;

UCLASS(Blueprintable, Abstract)
class OPENLOGICV2_API UCustomConnection : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
		void ConnectPins(UExecutionPinBase* InSourcePin, UExecutionPinBase* InTargetPin, bool IsPreviewConnection = false, bool ShouldSaveConnection = false);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void DisconnectPins();

private:
	UFUNCTION()
		void UpdateGraphDataConnection(bool bAddConnection);

	UFUNCTION()
		void UpdatePinConnectionData(TMap<int32, FOpenLogicPinState>& NodePins, int32 PinID, const FOpenLogicPinConnection& ConnectionData, bool bAddConnection);

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnConnectionInitialized();

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic", meta = (ForceAsFunction))
		void OnPaint(UPARAM(ref) FPaintContext& Context);

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void OnRecalculateConnection();

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool GetIsPreviewConnection() const { return bIsPreviewConnection; }

public:
	// Sets the graph editor that owns this connection.
	UFUNCTION()
		void SetOwningGraphEditor(UGraphEditorBase* InGraph) { OwningGraphEditor = InGraph; }

	// Gets the graph editor that owns this connection.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UGraphEditorBase* GetOwningGraphEditor() const { return OwningGraphEditor; }

public:
	// Gets the source pin of this connection.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UExecutionPinBase* GetSourcePin() const { return SourcePin; }

	// Gets the target pin of this connection.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UExecutionPinBase* GetTargetPin() const { return TargetPin; }

protected:
	UPROPERTY()
		UExecutionPinBase* SourcePin;

	UPROPERTY()
		UExecutionPinBase* TargetPin;

protected:
	// The graph editor that owns this connection.
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

protected:
	UPROPERTY()
		bool bIsPreviewConnection;

public:
	virtual UWorld* GetWorld() const override;
};
