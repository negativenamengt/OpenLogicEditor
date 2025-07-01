// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NodeBase.h"
#include "NodePaletteBase.h"
#include "Tasks/OpenLogicTask.h"
#include "Classes/OpenLogicGraph.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CanvasPanel.h"
#include "Components/ScaleBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "GraphEditorBase.generated.h"

// Forward declarations
class UCustomConnection;
class UConnectionRenderer;
class UGraphGridRenderer;
class UConnectionBase;
class UOpenLogicUtility;
class UGraphCustomization;
class UOpenLogicTaskCollection;
class UMenuAnchor;

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGraphEditorNewNodeDelegate, UNodeBase*, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGraphEditorSaveDelegate, FOpenLogicGraphData, New);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphEditorNodePositionDelegate, FVector2D, New, UNodeBase*, Node);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphEditorPositionChanged, FVector2D, OldPosition, FVector2D, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphEditorZoomLevelChanged, float, OldZoomLevel, float, NewZoomLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphEditorNodeSelect, UNodeBase*, Node, bool, IsSelected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphNodeDesiredSizeChanged, UNodeBase*, WidgetNode, FVector2D, NewSize);

UCLASS(Abstract)
class OPENLOGICV2_API UGraphEditorBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void InitializeGraphEditor(UOpenLogicGraph* NewGraphObject);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic")
		void BindGraphEditorInternalEvents();

protected:
	UFUNCTION()
		void OnNodeDesiredSizeChanged_Internal(UNodeBase* WidgetNode, FVector2D NewSize);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation);

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsInitialized() const { return bIsInitialized; }
	
private:
	UPROPERTY()
		bool bIsInitialized = false;

public:
	// Returns whether the graph editor runs inside the Unreal Editor or not.
	UFUNCTION(BlueprintPure, Category = OpenLogic)
		bool IsEditorGraph() const;

public:
	// Returns the active graph object.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UOpenLogicGraph* GetGraph() const;

protected:
	UPROPERTY()
		UOpenLogicGraph* ActiveGraph;

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Node")
		UNodeBase* GetNodeByGuid(FGuid NodeGuid);

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void SetOffsetOriginNode(UNodeBase* OriginNode);

	UFUNCTION(BlueprintPure, Category = "OpenLogic|Node")
		UNodeBase* GetOffsetOriginNode() const;

protected:
	UPROPERTY()
		UNodeBase* OffsetOriginNode;

private:
	UPROPERTY()
		bool bIsMovingNodes = false;

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		TArray<UNodeBase*> GetNodes() const;

private:
	// Mapping from GUID to Node, for quick access.
	UPROPERTY()
		TMap<FGuid, UNodeBase*> Nodes;

public:
	// Triggered when a new node is added to the graph editor.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Node|Dispatcher")
		FGraphEditorNewNodeDelegate OnNodeAdded;

	// Triggered when the graph is saved.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Saving")
		FGraphEditorSaveDelegate OnGraphSaved;

	// Triggered when a node position is changed by force.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Saving")
		FGraphEditorNodePositionDelegate OnNodePositionForceChange;

	// Delegate triggered when the desired size of a node has changed.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic")
		FGraphNodeDesiredSizeChanged OnNodeDesiredSizeChanged;

	// Triggered when the selection state of a node is changed.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Node|Dispatcher")
		FGraphEditorNodeSelect OnNodeSelectStateChanged;

public:
	// Triggered when the graph position has changed.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Saving")
		FGraphEditorPositionChanged OnGraphPositionChanged;

	// Triggered when the zoom level has changed.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|Saving")
		FGraphEditorZoomLevelChanged OnZoomLevelChanged;

public:
	UFUNCTION()
		void SetCurrentConnectionPreview(UCustomConnection* NewConnection);

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		UCustomConnection* GetCurrentPreviewConnection() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsPreviewConnectionActive() const;

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void DestroyConnectionPreview();

	UFUNCTION()
		void ClearAllConnections();

public:
	UPROPERTY(BlueprintReadWrite, Category = "OpenLogic|Connections", meta = (DeprecatedProperty, DeprecationMessage = "Direct access to CurrentConnectionPreview is deprecated. Please use the getter."))
		UCustomConnection* CurrentConnectionPreview;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		UCustomConnection* CreateConnection(UExecutionPinBase* SourcePin, UExecutionPinBase* TargetPin, bool IsPreviewConnection, bool ShouldSaveConnection = false);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void RemoveConnection(UCustomConnection* ConnectionToRemove);

	UFUNCTION()
		TArray<UCustomConnection*> GetActiveConnections() const;

private:
	UPROPERTY()
		TArray<UCustomConnection*> ActiveConnections;

public:
	// Saves the current graph position.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void SaveGraphPosition(FVector2D Position);

	// Sets the graph object to be used by the graph editor. This will clear all existing nodes.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void SetGraph(UOpenLogicGraph* NewGraph);

protected:
	UFUNCTION()
		void LoadNodes(TMap<FGuid, FOpenLogicNode>& GraphNodes);

	// Cleans up any unconnected pins from the node.
	// Typically called before loading a graph to ensure the graph data starts in a clean state.
	// Returns whether any cleanup was performed.
	UFUNCTION()
		bool CleanupNodeInvalidPins(const FGuid& NodeGuid);

private:
	UFUNCTION()
		bool CleanupPins_Internal(FOpenLogicNode& NodeData, TMap<int32, FOpenLogicPinState>& Pins, bool bInputPins);
	
public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		bool IsGraphLoaded() const { return bIsGraphLoaded; }

protected:
	UPROPERTY()
		bool bIsGraphLoaded = false;

public:
	// Sets the distance for snapping nodes. 0.0f will disable snapping.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void SetNodeSnapDistance(float NewSnapDistance);

	// Returns the distance for snapping nodes.
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Node")
		float GetNodeSnapDistance();

protected:
	// The distance for snapping nodes.
	UPROPERTY()
		float NodeSnapDistance = 0.0f;

protected:
	// Clears all the nodes in the graph editor.
	UFUNCTION()
		void ClearNodes();

	// Triggered when nodes are cleared from the graph editor (when ClearNodes is called).
	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic|Node")
		void OnClearNodesTriggered();

public:
	// Clears all the nodes and connections in the graph editor. This is called to reset the graph editor.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void ClearGraph();

public:
	// Adds a node to the selection list.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node|Selection")
		void AddSelectedNode(UNodeBase* NodeToSelect);

	// Sets the node selection list.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node|Selection")
		void SetSelectedNodes(const TArray<UNodeBase*>& NewSelectedNodes);

	// Removes a node from the selection list.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node|Selection")
		void RemoveSelectedNode(UNodeBase* NodeToDeselect);

	// Selects all the nodes in the graph editor.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node|Selection")
		void SelectAllNodes();

	// Clears the selection list.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node|Selection")
		void ClearSelectedNodes();

	// Returns all the selected nodes.
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Node|Selection")
		const TArray<UNodeBase*>& GetSelectedNodes() const
	{
		return SelectedNodes;
	}

	// Returns whether the node is selected.
	UFUNCTION(BlueprintPure, Category = "OpenLogic|Node|Selection")
		bool IsNodeSelected(UNodeBase* NodeToCheck) const
	{
		return SelectedNodes.Contains(NodeToCheck);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void AlignSelectedNodesWithOrigin();

protected:
	// A list of all the selected nodes.
	UPROPERTY()
		TArray<UNodeBase*> SelectedNodes;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Timer")
		void RegisterTimer(FTimerHandle Handle);

	// Used to clean up all timers.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Timer")
		void ClearAllTimers();

private:
	// List of all the timers that are currently active.
	UPROPERTY()
		TArray<FTimerHandle> Timers;

protected:
	// The node palette class spawned by the graph editor.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "OpenLogic", meta = (ExposeOnSpawn = true))
		TSubclassOf<UNodePaletteBase> NodePaletteClass = TSoftClassPtr<UNodePaletteBase>(FSoftObjectPath(TEXT("/OpenLogicV2/OpenLogicEditor/NodePalette/WBP_DefaultNodePalette.WBP_DefaultNodePalette_C"))).Get();
	
	// The current node palette widget.
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|NodePalette")
		UNodePaletteBase* NodePaletteWidget;

public:
	// Sets the node palette class to be used by the graph editor.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
		void SetNodePalette(TSubclassOf<UNodePaletteBase> NewNodePaletteClass);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
		UNodePaletteBase* GetNodePaletteWidget() const { return NodePaletteWidget; };

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "OpenLogic|NodePalette")
		void ShowNodePalette(UNodePaletteBase* NodePalette);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "OpenLogic|NodePalette")
		void HideNodePalette(UNodePaletteBase* NodePalette);

	UFUNCTION(BlueprintImplementableEvent, Category = "OpenLogic|NodePalette")
		void OnNodePaletteChanged(UNodePaletteBase* NewNodePaletteWidget);

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OpenLogic", meta = (ExposeOnSpawn = "true"))
		TArray<UOpenLogicTaskCollection*> TaskCollections;

private:
	// Creates a node widget based on the Task Class and caches it.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		UNodeBase* NewNodeReference(TSubclassOf<UOpenLogicTask> TaskClass, bool SaveNode, FGuid LoadID);

public:
	// Creates a node widget based on the Task Class.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		UNodeBase* CreateNodeReference(TSubclassOf<UOpenLogicTask> TaskClass);

public:
	// Creates a node widget and adds it to the graph editor.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		UNodeBase* CreateNode(TSubclassOf<UOpenLogicTask> TaskClass, FVector2D Position, bool SaveNode = true, FGuid LoadID = FGuid());

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void DeleteNode(UNodeBase* NodeToDelete);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|Node")
		void DeleteNodes(const TArray<UNodeBase*>& NodesToDelete);

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void BindRuntimeWorker(UOpenLogicRuntimeGraph* NewRuntimeWorker);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void UnbindRuntimeWorker();

private:
	UFUNCTION()
		void OnRuntimeTaskActivated(UOpenLogicTask* ActivatedRuntimeTask);

	UFUNCTION()
		void OnRuntimeTaskCompleted(UOpenLogicTask* CompletedRuntimeTask);

private:
	UPROPERTY()
		UOpenLogicRuntimeGraph* BoundRuntimeWorker;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		bool SetZoomLevel(float NewZoomLevel);

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		float GetZoomLevel() const;

protected:
	UPROPERTY()
		float MinZoomLevel = 0.6f;

	UPROPERTY()
		float MaxZoomLevel = 2.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void SetGraphPosition(FVector2D NewPosition, bool UpdateGridPosition = true);

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FVector2D GetGraphPosition() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FVector2D AbsoluteToGraphPosition(FVector2D AbsolutePosition) const;

protected:
	UPROPERTY()
		FVector2D CurrentGraphPosition = FVector2D::Zero();

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FVector2D GetMousePosition() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		const FVector2D GetGraphMousePosition() const;

protected:
	UFUNCTION()
		void Timer_SaveGraphPosition();

protected:
	UPROPERTY()
		bool bCanSaveGraphPosition = false;

public:
	// Sets the position of the grid material.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void SetGridPosition(FVector2D NewPosition);

	// Returns the position of the grid material.
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		FVector2D GetGridPosition() const;

private:
	UPROPERTY()
		FVector2D GridOffset = FVector2D::ZeroVector;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void RefreshStyle();

private:
	// The size of each cell in the grid.
	UPROPERTY()
		int32 CellSize = 1200;

protected:
	// A map of all the grid cells.
	UPROPERTY()
		TMap<FIntPoint, FOpenLogicGridCell> GridCells;

public:
	UFUNCTION()
		void AddNodeToCell(UNodeBase* NodeToAdd, FIntPoint CellPosition);

	UFUNCTION()
		void RemoveNodeFromCell(UNodeBase* NodeToRemove, FIntPoint CellPosition);

	UFUNCTION()
		FIntPoint GetCellPosition(FVector2D Position) const;

	// Returns all the cells that are occupied by nodes.
	UFUNCTION()
		TArray<FIntPoint> GetAllActiveCells() const;

	// Returns the size of a cell.
	UFUNCTION()
		int32 GetCellSize() const;

	UFUNCTION()
		const TSet<UNodeBase*> GetNodesInCell(FIntPoint CellPosition) const;

	// Returns the cells that intersect from the start to the end.
	UFUNCTION()
		TArray<FIntPoint> GetBoxIntersectingCells(FVector2D Start, FVector2D End, bool IncludeEmpty) const;

	// Returns the cells that are within the visible area of the graph editor.
	UFUNCTION()
		TArray<FIntPoint> GetVisibleCells() const;

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidget))
		UImage* Image_Background;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidget))
		UImage* Image_GraphGrid;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidget))
		UScaleBox* ScaleBox_Graph;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidget))
		UCanvasPanel* CanvasPanel_Nodes;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidget))
		UMenuAnchor* MenuAnchor_NodePalettePanel;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidgetOptional))
		UImage* SelectionBox;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidgetOptional))
		UConnectionRenderer* ConnectionRenderer;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|Widgets", meta = (BindWidgetOptional))
		UGraphGridRenderer* GridRenderer;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = "OpenLogic", meta = (ExposeOnSpawn))
		UGraphCustomization* GraphStyleAsset;

private:
	UPROPERTY()
		UMaterialInstanceDynamic* GridDynamicMaterial;

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
		void SetTasksToLoad(const TArray<TSoftClassPtr<UOpenLogicTask>>& NewTasks);

	UFUNCTION(BlueprintPure, Category = "OpenLogic|NodePalette")
		TArray<TSoftClassPtr<UOpenLogicTask>> GetTasksToLoad() const;

private:
	// Used by the node palette to fetch the tasks.
	UPROPERTY()
		TArray<TSoftClassPtr<UOpenLogicTask>> TasksToLoad;
};