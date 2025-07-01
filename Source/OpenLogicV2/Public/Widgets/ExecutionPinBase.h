// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "Blueprint/UserWidget.h"
#include "Tasks/OpenLogicTask.h"
#include "Classes/CustomConnection.h"
#include "Classes/DefaultNodeConnection.h"
#include "Components/Image.h"
#include "ExecutionPinBase.generated.h"

class UGraphEditorBase;
class UNodeBase;
class UOpenLogicProperty;
struct FOpenLogicPinState;

UCLASS(Abstract)
class OPENLOGICV2_API UExecutionPinBase : public UUserWidget
{
	GENERATED_BODY()
public:
	// If valid, GetGraphPosition() will return the position of this widget. Otherwise, it will return the position of the main widget.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic, meta = (BindWidgetOptional))
		UWidget* InPinWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = OpenLogic)
		bool bShouldCreateDefaultValueWidget = true;
	
public:
	// Sets the node that this pin belongs to. This is only set when the pin is created.
	UFUNCTION()
		void SetOwningNode(UNodeBase* Node) { OwningNode = Node; }

	// Returns the node that this pin belongs to.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UNodeBase* GetOwningNode() const { return OwningNode; }

	// Sets the graph editor that owns this pin. This is only set when the pin is created.
	UFUNCTION()
		void SetOwningGraphEditor(UGraphEditorBase* GraphEditor) { OwningGraphEditor = GraphEditor; }

	// Returns the graph editor that owns this pin.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UGraphEditorBase* GetOwningGraphEditor() const { return OwningGraphEditor; }

protected:
	// The node that this pin belongs to.
	UPROPERTY()
		UNodeBase* OwningNode;

	// The graph editor that owns this pin.
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		const TArray<UCustomConnection*>& GetConnections() const { return LinkConnection; }

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool HasAnyConnections() const { return LinkConnection.Num() > 0; }

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic", meta = (DeprecatedProperty, DeprecationMessage = "Direct access to LinkConnection is deprecated. Please use the getter."))
		TArray<UCustomConnection*> LinkConnection;

public:
	UFUNCTION()
		void SetPinID(int32 NewPinID) { PinID = NewPinID; }
	
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		int32 GetPinID() const { return PinID; }

	UFUNCTION()
		void SetPinInfo(FOpenLogicPinData NewPinInfo) { PinInfo = NewPinInfo; }
	
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FOpenLogicPinData GetPinInfo() const { return PinInfo; }
	
private:
	UPROPERTY()
		int32 PinID = INDEX_NONE;

	UPROPERTY()
		FOpenLogicPinData PinInfo;

public:
	UFUNCTION()
		void SetPinDirection(EOpenLogicPinDirection NewPinDirection) { PinDirection = NewPinDirection; }
	
public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsInputPin() const { return PinDirection == EOpenLogicPinDirection::Input; }

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsOutputPin() const { return PinDirection == EOpenLogicPinDirection::Output; }

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		EOpenLogicPinDirection GetPinDirection() const { return PinDirection; }

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool IsFlowControlPin() const { return PinInfo.Role == EPinRole::FlowControl; }

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool IsDataPropertyPin() const { return PinInfo.Role == EPinRole::DataProperty; }

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		const FText GetFriendlyPinToolTipText() const;

public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		TSubclassOf<UOpenLogicProperty> GetPropertyClass() const;

	// If bUseResolvedType is true, the resolved property will be used if it exists.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UOpenLogicProperty* GetPropertyDefaultObject() const;
	
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FLinearColor GetPinColor(FLinearColor FallbackColor = FLinearColor::White) const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FText GetPinDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FText GetPinDescription() const { return PinInfo.Description; }

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		EPinRole GetPinRole() const;
	
private:
	UPROPERTY()
		EOpenLogicPinDirection PinDirection;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void BeginConnectionPreview();

public:
	// Tries to remove the pin. Only works with dynamically added pins.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void TryRemovePin();

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void Unlink();

public:
	void OnConnected(UExecutionPinBase* Other, UCustomConnection* ConnectionWidget);
	void OnUnlink();
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic")
		void ReceiveOnConnected(UExecutionPinBase* Other, UCustomConnection* ConnectionWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic")
		void ReceiveOnUnlink();

	// Called when a connection has been started from this pin.
	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic")
		void OnConnectionPreviewStarted();

	// Called when a connection has been ended from this pin.
	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic")
		void OnConnectionPreviewEnded();

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		void ReceivePinData(FOpenLogicPinData PinInformation);

	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		void ReceiveDefaultValueWidget(const FOpenLogicPinValueDisplay& DisplayData);

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		EPinConnectionValidity CanConnectTo(UExecutionPinBase* Other) const;

	// Returns true if the pin is replaceable.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		const bool IsReplaceable() const;

public:
	// Returns the centered position of the pin in the graph editor.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FVector2D GetGraphPosition() const;

public:
	UFUNCTION()
		void CreateDefaultValueWidget();

	UFUNCTION()
		void DestroyDefaultValueWidget();

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UWidget* GetDefaultValueWidget() const { return DefaultValueWidget; }

	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SetDefaultValue(FOpenLogicDefaultValue NewDefaultValue);

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FOpenLogicDefaultValue GetDefaultValue() const;

public:
	UFUNCTION()
		void ResolvePin(TSubclassOf<UOpenLogicProperty> PropertyClass);

	UFUNCTION()
		void ResetPinType();
	
private:
	UPROPERTY()
		UWidget* DefaultValueWidget;

	UPROPERTY()
		TSubclassOf<UOpenLogicProperty> ResolvedPropertyClass;

private:
	void Internal_RemovePin(TMap<int32, UExecutionPinBase*>& PinMap, TMap<int32, FOpenLogicPinState>& SavePinMap);
};
