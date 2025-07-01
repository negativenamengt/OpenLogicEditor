// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "Blueprint/UserWidget.h"
#include "Tasks/OpenLogicTask.h"
#include "Components/Widget.h"
#include "Components/TextBlock.h"
#include "ExecutionPinBase.h"
#include "NodeBase.generated.h"

// Forward declarations
class UGraphEditorBase;
class UConnectionBase;
class UOpenLogicPayloadWidget;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRuntimeTaskActivated, UOpenLogicTask*, RuntimeTask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRuntimeTaskCompleted, UOpenLogicTask*, RuntimeTask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPayloadPropertyChanged, FName, PropertyName);

static FOpenLogicNode EmptyNodeState;

UCLASS(BlueprintType, Abstract)
class OPENLOGICV2_API UNodeBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		FTaskData InitializeNode(UTextBlock* NameTextBlock, UTextBlock* TypeTextBlock);

public:
	// Called when the node has been registered by the owning graph editor.
	UFUNCTION(BlueprintImplementableEvent, Category = OpenLogic)
		void OnNodeRegistered();

	// Called when the node has been unregistered by the owning graph editor. Usually when the node is cleared from the graph or deleted.
	UFUNCTION(BlueprintImplementableEvent, Category = OpenLogic)
		void OnNodeUnregistered();

	UFUNCTION(BlueprintImplementableEvent, Category = OpenLogic)
		void OnPinAdded(UExecutionPinBase* Pin);

	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		FVector2D GetPinLocalPosition(const UExecutionPinBase* Pin);

	// Native helper function to compute the local position of a pin vertically.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		FVector2D ComputePinLocalPosition(const UExecutionPinBase* Pin, FVector2D StartOffset);

	// Native helper to compute the local position of a pin in behavior-tree style. (Top-Center/Bottom-Center)
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		FVector2D ComputePinLocalPosition_BehaviorTree(const UExecutionPinBase* Pin, FVector2D StartOffset, const float PinSpacing);

	// Native helper function to get the top offset of a pin.
	UFUNCTION(BlueprintCallable)
		float ComputePinTopOffset(const UExecutionPinBase* Pin);

	// Native helper to get the horizontal offset of a pin.
	UFUNCTION()
		float ComputePinHorizontalOffset(const UExecutionPinBase* Pin, const float PinSpacing, bool bCenterAlign) const;

public:
	// Sets the graph editor that owns this node.
	UFUNCTION()
		void SetOwningGraphEditor(UGraphEditorBase* InGraphEditor);

	// Returns the graph editor that owns this node.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UGraphEditorBase* GetOwningGraphEditor() const;

private:
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SetNodePosition(const FVector2D Position);

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FVector2D GetNodePosition() const;

public:
	// Sets the position where the selection started.
	UFUNCTION()
		void SetSelectionStartPosition(FVector2D Position);

	// Gets the position where the selection started.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FVector2D GetSelectionStartPosition() const;

protected:
	// The position where the selection started.
	UPROPERTY()
		FVector2D SelectionStartPosition;

public:
	// Refreshes the cells that this node occupies.
	UFUNCTION()
		void UpdateNodeCells();

public:
	// Returns the position where the mouse was clicked down. Used for moving the node.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FVector2D GetMouseDownPosition() const { return MouseDownPosition; }

protected:
	// The position where the mouse was clicked down.
	UPROPERTY()
		FVector2D MouseDownPosition;

public:
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic, meta = (ExposeOnSpawn = true))
		TSubclassOf<UOpenLogicTask> TaskClass;

	// Instance of the task class.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		UOpenLogicTask* TaskObject;

public:
	// Instance of the UOpenLogicTask class created at runtime.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		UOpenLogicTask* RuntimeTaskObject;

	UPROPERTY(BlueprintAssignable, Category = OpenLogic)
		FRuntimeTaskActivated OnRuntimeTaskActivated;

	UPROPERTY(BlueprintAssignable, Category = OpenLogic)
		FRuntimeTaskCompleted OnRuntimeTaskCompleted;

public:
	// Contains the data associated with the task.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		FTaskData TaskData;

public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FGuid GetNodeID() const { return NodeID; }

public:
	// The unique identifier for the node.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		FGuid NodeID;

public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		FOpenLogicNode& GetNodeStateData() const;
	
public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		int32 GetInputPinsCount() const { return InputPins.Num(); }

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		int32 GetOutputPinsCount() const { return OutputPins.Num(); }
	
public:
	// A boolean indicating whether the pins for this node have been loaded.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		bool HasLoadedPins;

public:
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		TArray<UExecutionPinBase*> GetAllPins() const;

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UExecutionPinBase* GetInputPinByName(const FName& PinName) const;

	UFUNCTION(BlueprintPure, Category = OpenLogic)
		UExecutionPinBase* GetOutputPinByName(const FName& PinName) const;

public:
	// A map of input pins for the node, with the integer as the pin index.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		TMap<int32, UExecutionPinBase*> InputPins;

	// A map of output pins for the node, with the integer as the pin index.
	UPROPERTY(BlueprintReadOnly, Category = OpenLogic)
		TMap<int32, UExecutionPinBase*> OutputPins;

public:
	// The pin class to use when creating pins for this node.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = OpenLogic)
		TSubclassOf<UExecutionPinBase> PinClass;

	// The pin connection class to use when creating connections between pins.
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = OpenLogic)
		TSubclassOf<UCustomConnection> PinConnectionClass = UDefaultNodeConnection::StaticClass();

public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void AddDisplayableWidget(TSubclassOf<UDisplayableWidgetBase> WidgetClass);

	UFUNCTION(BlueprintImplementableEvent, Category = OpenLogic)
		void OnDisplayableWidgetAdded(UWidget* Widget);

public:
	// Used for tracking the offset of the node during movement.
	UPROPERTY(BlueprintReadWrite, Category = OpenLogic)
		FVector2D MovingOffset;

public:
	// Recalculate the positions and directions of the connections.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void RecalculateConnections() const;

public:
	// Disconnects the specified connections from this node.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void BreakPinsConnections(TArray<UExecutionPinBase*> PinConnectionsToBreak);

	// Disconnects all the pins from this node.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void BreakAllPinsConnections();

public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		UExecutionPinBase* AddInputPin(FOpenLogicPinData PinData, bool IsUserCreated);

	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		UExecutionPinBase* AddOutputPin(FOpenLogicPinData PinData, bool IsUserCreated);

private:
	UFUNCTION()
		UExecutionPinBase* AddPin(FOpenLogicPinData PinData, bool IsUserCreated, EOpenLogicPinDirection PinDirection);

public:
	UFUNCTION()
		void ResolvePinTypes(TSubclassOf<UOpenLogicProperty> PropertyClass);

	UFUNCTION()
		void ResetPinTypes();

public:
	// Schedule a connection update based on the duration of the last frame.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void ScheduleConnectionUpdate();

public:
	// Load the pins for this node.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void LoadPins();

	// Load the displayable widgets for this node.
	UFUNCTION()
		void LoadWidgets();

	// Load the task properties for this node.
	UFUNCTION()
		void LoadTaskProperties();

	// Load the given task property for this node.
	UFUNCTION()
		bool LoadTaskProperty(const FName& PropertyName, const FString& PropertyValue) const;

public:
	// Save the node state data.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SaveNode();

	// Save the position of the node.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SaveNodePosition(FVector2D Position);
	
	// Saves all the task properties. Please use this function only when necessary, otherwise, use SaveTaskProperty.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SaveTaskProperties();

	// Save the given task property.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		void SaveTaskProperty(FName PropertyName);

public:
	// Delete this node from the graph.
	UFUNCTION(BlueprintCallable, Category = OpenLogic, meta = (DeprecatedFunction, DeprecationMessage = "Please use GraphEditor::DeleteNode instead."))
		void DeleteNode();

public:
	// Called when the node is selected.
	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		void OnNodeSelected();

	// Called when the node is deselected.
	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		void OnNodeDeselected();

public:
	// Delegate triggered when a payload property has changed.
	UPROPERTY(BlueprintAssignable, Category = OpenLogic)
		FPayloadPropertyChanged OnPayloadPropertyChanged;

protected:
	// All the cells that this node occupies.
	UPROPERTY()
		TArray<FIntPoint> NodeCells;
};