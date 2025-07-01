// Copyright 2024 - NegativeNameSeller

#include "Widgets/GraphEditorBase.h"
#include "Classes/CustomConnection.h"
#include "Classes/OpenLogicGraphEditorManager.h"
#include "Widgets/ConnectionRenderer.h"
#include "Widgets/GraphGridRenderer.h"
#include "Utility/OpenLogicUtility.h"
#include "Tasks/OpenLogicTask.h"
#include "Classes/GraphCustomization.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/MenuAnchor.h"
#include "Brushes/SlateNoResource.h"
#include "Framework/Application/SlateApplication.h"
#include "Classes/GraphSelectionDragOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "TimerManager.h"
#include "OpenLogicV2.h"

void UGraphEditorBase::InitializeGraphEditor(UOpenLogicGraph* NewGraphObject)
{
    if (bIsInitialized)
    {
        UE_LOG(OpenLogicLog, Warning, TEXT("[UGraphEditorBase::InitializeGraphEditor] This graph editor is already initialized."))
        return;
    }

    // Update the graph object
    ActiveGraph = NewGraphObject;

    // Bind the blueprint events
    BindGraphEditorInternalEvents();
	
	// Bind the save graph position timer
	FTimerHandle TimerHandle_SaveGraphPosition;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_SaveGraphPosition,
		this,
		&UGraphEditorBase::Timer_SaveGraphPosition,
		1.0f,
		true
	);
	
	RegisterTimer(TimerHandle_SaveGraphPosition);

    // Bind the internal node desired size changed event
    OnNodeDesiredSizeChanged.AddDynamic(this, &UGraphEditorBase::OnNodeDesiredSizeChanged_Internal);

    // Give the reference of the graph editor to the connection renderer
    if (ConnectionRenderer)
    {
        ConnectionRenderer->OwningGraphEditor = this;
    }

    // Give the reference of the graph editor to the grid renderer
    if (GridRenderer)
	{
		GridRenderer->OwningGraphEditor = this;
	}

    // Update the node palette class
    SetNodePalette(NodePaletteClass);

    // Load the graph
    SetGraph(NewGraphObject);

    // Mark the graph editor as initialized
    bIsInitialized = true;
}

void UGraphEditorBase::OnNodeDesiredSizeChanged_Internal(UNodeBase* WidgetNode, FVector2D NewSize)
{
    if (!WidgetNode)
    {
        return;
    }

    WidgetNode->ScheduleConnectionUpdate();
    WidgetNode->UpdateNodeCells();
}

void UGraphEditorBase::NativeOnInitialized()
{
    if (MenuAnchor_NodePalettePanel)
    {
        // Set whether the node palette panel should use the application stack.
        MenuAnchor_NodePalettePanel->UseApplicationMenuStack = GIsEditor && !GIsPlayInEditorWorld;
    }
}

void UGraphEditorBase::NativeConstruct()
{
    Super::NativeConstruct();

    UOpenLogicGraphEditorManager::Get()->AddGraphEditor(this);
}

void UGraphEditorBase::NativeDestruct()
{
    Super::NativeDestruct();

    UOpenLogicGraphEditorManager::Get()->RemoveGraphEditor(this);
}

void UGraphEditorBase::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!SelectionBox)
	{
		return;
	}

    OutOperation = UWidgetBlueprintLibrary::CreateDragDropOperation(UGraphSelectionDragOperation::StaticClass());

    UGraphSelectionDragOperation* DragOperation = Cast<UGraphSelectionDragOperation>(OutOperation);
    DragOperation->Initialize(this);
}

bool UGraphEditorBase::IsEditorGraph() const
{
	return GetOwningPlayer() == nullptr;
}

UOpenLogicGraph* UGraphEditorBase::GetGraph() const
{
    return ActiveGraph;
}

UNodeBase* UGraphEditorBase::GetNodeByGuid(FGuid NodeGuid)
{
    if (Nodes.Contains(NodeGuid))
    {
		return Nodes[NodeGuid];
	}
	return nullptr;
}

void UGraphEditorBase::SetOffsetOriginNode(UNodeBase* OriginNode)
{
    if (!OriginNode && IsValid(OffsetOriginNode))
    {
        OffsetOriginNode = nullptr;

        for (UNodeBase* SelectedNode : SelectedNodes)
        {
        	if (SelectedNode->GetNodeStateData().Position == SelectedNode->GetNodePosition())
        		continue;
        	
            SelectedNode->SaveNodePosition(SelectedNode->GetNodePosition());
        }

        return;
    }

    OffsetOriginNode = OriginNode;
}

UNodeBase* UGraphEditorBase::GetOffsetOriginNode() const
{
    return OffsetOriginNode;
}

TArray<UNodeBase*> UGraphEditorBase::GetNodes() const
{
    TArray<UNodeBase*> NodeWidgets;
	Nodes.GenerateValueArray(NodeWidgets);

	return NodeWidgets;
}

void UGraphEditorBase::SetCurrentConnectionPreview(UCustomConnection* NewConnection)
{
    CurrentConnectionPreview = NewConnection;
}

UCustomConnection* UGraphEditorBase::GetCurrentPreviewConnection() const
{
    return CurrentConnectionPreview;
}

bool UGraphEditorBase::IsPreviewConnectionActive() const
{
    return IsValid(CurrentConnectionPreview);
}

void UGraphEditorBase::DestroyConnectionPreview()
{
    if (!IsPreviewConnectionActive())
    {
        return;
    }

    UCustomConnection* ConnectionPreview = this->GetCurrentPreviewConnection();

    ActiveConnections.Remove(ConnectionPreview);
    ConnectionPreview->GetSourcePin()->OnConnectionPreviewEnded();
    
    ConnectionPreview->MarkAsGarbage();
    ConnectionPreview = nullptr;
}

void UGraphEditorBase::ClearAllConnections()
{
    TArray<UCustomConnection*> ConnectionsToRemove;
    ConnectionsToRemove.Append(GetActiveConnections());

	for (UCustomConnection* Connection : ConnectionsToRemove)
	{
        Connection->MarkAsGarbage();
	}

    ActiveConnections.Empty();
}

UCustomConnection* UGraphEditorBase::CreateConnection(UExecutionPinBase* SourcePin, UExecutionPinBase* TargetPin, bool IsPreviewConnection, bool ShouldSaveConnection)
{
    TSubclassOf<UCustomConnection> ConnectionClass = SourcePin->GetOwningNode()->PinConnectionClass;
    check(ConnectionClass);

    if (!IsPreviewConnection)
    {
        if (SourcePin->IsInputPin() && TargetPin->IsOutputPin())
        {
            UExecutionPinBase* TempPin = SourcePin;
			SourcePin = TargetPin;
			TargetPin = TempPin;
        }

        EPinConnectionValidity ConnectionValidity = SourcePin->CanConnectTo(TargetPin);
        if (ConnectionValidity != EPinConnectionValidity::Compatible && ConnectionValidity != EPinConnectionValidity::Replaceable)
		{
			return nullptr;
		}

        if (ConnectionValidity == EPinConnectionValidity::Replaceable)
		{
            UExecutionPinBase* PinToReplace = SourcePin->IsReplaceable() ? SourcePin : TargetPin;

            if (PinToReplace)
            {
                PinToReplace->Unlink();
            }
		}
    }

    UCustomConnection* NewConnection = NewObject<UCustomConnection>(this, ConnectionClass);

    // Initialize the properties of the new connection
    NewConnection->SetOwningGraphEditor(this);
    NewConnection->ConnectPins(SourcePin, TargetPin, IsPreviewConnection, ShouldSaveConnection);

    if (IsPreviewConnection)
	{
		SetCurrentConnectionPreview(NewConnection);
	}

	ActiveConnections.Add(NewConnection);

	NewConnection->OnConnectionInitialized();
	NewConnection->OnRecalculateConnection();

    return NewConnection;
}

void UGraphEditorBase::RemoveConnection(UCustomConnection* ConnectionToRemove)
{
    if (!IsValid(ConnectionToRemove))
	{
		return;
	}

	ConnectionToRemove->DisconnectPins();
    ActiveConnections.Remove(ConnectionToRemove);
}

TArray<UCustomConnection*> UGraphEditorBase::GetActiveConnections() const
{
    return ActiveConnections;
}

void UGraphEditorBase::SaveGraphPosition(FVector2D Position)
{
    if (!GetGraph())
	{
		return;
	}

    GetGraph()->GraphData.GraphPosition = Position;

    #if WITH_EDITOR
    ActiveGraph->Modify();
    #endif

    OnGraphSaved.Broadcast(GetGraph()->GraphData);
}

void UGraphEditorBase::AlignSelectedNodesWithOrigin()
{
    if (!OffsetOriginNode)
    {
        return;
    }

    FVector2D OriginSelectionStartPos = OffsetOriginNode->GetSelectionStartPosition();
    FVector2D OriginCurrentPosition = OffsetOriginNode->GetNodePosition();

    FVector2D Offset = OriginCurrentPosition - OriginSelectionStartPos;

    for (UNodeBase* SelectedNode : SelectedNodes)
	{
		if (SelectedNode == OffsetOriginNode)
		{
			continue;
		}

		FVector2D NewPosition = SelectedNode->GetSelectionStartPosition() + Offset;
		SelectedNode->SetNodePosition(NewPosition);
	}
}

void UGraphEditorBase::RegisterTimer(FTimerHandle Handle)
{
    Timers.Add(Handle);
}

void UGraphEditorBase::ClearAllTimers()
{
    /* Clears all timer handles */
    for (FTimerHandle Handle : Timers)
    {
        GetWorld()->GetTimerManager().ClearTimer(Handle);
        Timers.Remove(Handle);
    }
}

void UGraphEditorBase::SetGraph(UOpenLogicGraph* NewGraph)
{
    if (!IsValid(NewGraph))
    {
    	UE_LOG(OpenLogicLog, Warning, TEXT("GraphToLoad is invalid or null."));
        return;
    }

    // Clean up the graph editor before loading a new graph
    ClearGraph();
    ActiveGraph = NewGraph;

	GraphStyleAsset = ActiveGraph->GraphCustomization.LoadSynchronous();
	if (!GraphStyleAsset)
	{
		GraphStyleAsset = UOpenLogicGraph::GetDefaultGraphCustomization().LoadSynchronous();
	}
	
	RefreshStyle();

    // Mark the graph as loading
    this->bIsGraphLoaded = false;

	// Migration to the latest schema version
	ActiveGraph->MigrateToLatestSchemaVersion();

    // Load the graph position
    const FVector2D& GraphPosition = ActiveGraph->GraphData.GraphPosition;
    this->SetGraphPosition(GraphPosition);
    bCanSaveGraphPosition = false;

    // Reserve space for node IDs to avoid multiple reallocations
    Nodes.Reserve(GetGraph()->GraphData.Nodes.Num());

    // Load the nodes and connections
    LoadNodes(ActiveGraph->GraphData.Nodes);

    // Mark the graph as loaded
    this->bIsGraphLoaded = true;
}

void UGraphEditorBase::LoadNodes(TMap<FGuid, FOpenLogicNode>& GraphNodes)
{
	TArray<UNodeBase*> NodesWithOutputPins;

	bool bHasModifiedGraphData = false;
	
	for (auto It = GraphNodes.CreateIterator(); It; ++It)
	{
		const FGuid& NodeGUID = It.Key();
		const FOpenLogicNode& NodeData = It.Value();

		TSubclassOf<UOpenLogicTask> TaskClass;

		if (NodeData.TaskClass.IsPending())
		{
			TaskClass = NodeData.TaskClass.LoadSynchronous();
		} else if (NodeData.TaskClass.IsValid())
		{
			TaskClass = NodeData.TaskClass.Get();
		}

		if (!TaskClass)
		{
			bHasModifiedGraphData |= CleanupNodeInvalidPins(NodeGUID);

			// Remove the node from the graph data if the task class is invalid
			It.RemoveCurrent();
			continue;
		}

		UNodeBase* NodeWidget = NewNodeReference(NodeData.TaskClass.LoadSynchronous(), false, NodeGUID);
		if (!NodeWidget)
		{
			continue;
		}

		// Store the node in the nodes map
		Nodes.Add(NodeGUID, NodeWidget);

		// Set the node position
		NodeWidget->SetNodePosition(NodeData.Position);

		// Broadcast the node registered event
		NodeWidget->OnNodeRegistered();

		// Perform cleanup of unconnected pins for the node
		bHasModifiedGraphData |= CleanupNodeInvalidPins(NodeGUID);
		
		// Check for output pins and store in array if found
		if (NodeData.OutputPins.Num() > 0)
		{
			NodesWithOutputPins.Add(NodeWidget);
		}
	}

	auto& GraphEvents = GetGraph()->GraphData.Events;

	for (auto It = GraphEvents.CreateIterator(); It; ++It)
	{
		const TSubclassOf<UOpenLogicTask>& TaskClass = It.Key();
		FOpenLogicEventContainer& EventContainer = It.Value();

		if (!IsValid(TaskClass))
		{
			It.RemoveCurrent();
			bHasModifiedGraphData = true;
			continue;
		}
		
		for (int i = EventContainer.NodeId.Num() - 1; i >= 0; i--)
		{
			const FGuid& NodeId = EventContainer.NodeId[i];

			if (!GetGraph()->GraphData.Nodes.Contains(NodeId))
			{
				EventContainer.NodeId.RemoveAt(i);
				bHasModifiedGraphData = true;
			}
		}

		if (EventContainer.NodeId.Num() == 0)
		{
			It.RemoveCurrent();
			bHasModifiedGraphData = true;
		}
	}

	for (UNodeBase* Node : NodesWithOutputPins)
	{
		auto OutputPins = Node->GetNodeStateData().OutputPins;
		for (const auto OutputPinPair : OutputPins)
		{
			const int32 PinIndex = OutputPinPair.Key;
			const FOpenLogicPinState& PinState = OutputPinPair.Value;

			for (const FOpenLogicPinConnection Connection : PinState.Connections)
			{
				UExecutionPinBase* SourcePin = Node->OutputPins.FindRef(PinIndex);
				if (!SourcePin)
				{
					continue;
				}

				if (!Nodes.Contains(Connection.NodeID))
				{
					continue;
				}
				
				UNodeBase* TargetNode = Nodes[Connection.NodeID];
				if (!TargetNode)
				{
					continue;
				}

				UExecutionPinBase* TargetPin = TargetNode->InputPins.FindRef(Connection.PinID);
				if (!TargetPin)
				{
					continue;
				}

				CreateConnection(SourcePin, TargetPin, false, false);
			}
		}
	}

	// Check if the graph data has changed
	if (bHasModifiedGraphData)
	{
		// Mark the graph as modified
		GetGraph()->Modify();
		OnGraphSaved.Broadcast(GetGraph()->GraphData);
	}
}

bool UGraphEditorBase::CleanupNodeInvalidPins(const FGuid& NodeGuid)
{
	if (!NodeGuid.IsValid() || !GetGraph()->GraphData.Nodes.Contains(NodeGuid))
	{
		return false;
	}

	FOpenLogicNode& NodeData = GetGraph()->GraphData.Nodes[NodeGuid];

	bool bHasModifiedOutputPins = CleanupPins_Internal(NodeData, NodeData.OutputPins, false);
	bool bHasModifiedInputPins = CleanupPins_Internal(NodeData, NodeData.InputPins, true);

	return bHasModifiedOutputPins || bHasModifiedInputPins;
}

bool UGraphEditorBase::CleanupPins_Internal(FOpenLogicNode& NodeData, TMap<int32, FOpenLogicPinState>& Pins, bool bInputPins)
{
    if (Pins.Num() == 0)
    {
        return false;
    }

    bool bHasModifiedPins = false;

    UClass* SourceTaskClass = nullptr;
    if (!NodeData.TaskClass.IsNull())
    {
        SourceTaskClass = NodeData.TaskClass.Get();
        if (!SourceTaskClass)
        {
            SourceTaskClass = NodeData.TaskClass.LoadSynchronous();
        }
    }

    for (auto It = Pins.CreateIterator(); It; ++It)
    {
        FOpenLogicPinState& PinState = It.Value();

        for (int i = PinState.Connections.Num() - 1; i >= 0; --i)
        {
            const FOpenLogicPinConnection& PinConnection = PinState.Connections[i];

            if (!GetGraph()->GraphData.Nodes.Contains(PinConnection.NodeID))
            {
                PinState.Connections.RemoveAt(i);
                bHasModifiedPins = true;
                continue;
            }

            FOpenLogicNode& TargetNode = GetGraph()->GraphData.Nodes[PinConnection.NodeID];
            auto& TargetNodePins = bInputPins ? TargetNode.OutputPins : TargetNode.InputPins;

            if (!TargetNodePins.Contains(PinConnection.PinID))
            {
                PinState.Connections.RemoveAt(i);
                bHasModifiedPins = true;
                continue;
            }

            UClass* TargetTaskClass = nullptr;
            if (!TargetNode.TaskClass.IsNull())
            {
                TargetTaskClass = TargetNode.TaskClass.Get();
                if (!TargetTaskClass)
                {
                    TargetTaskClass = TargetNode.TaskClass.LoadSynchronous();
                }
            }

            if (!SourceTaskClass || !TargetTaskClass)
            {
                PinState.Connections.RemoveAt(i);
                bHasModifiedPins = true;
                continue;
            }

            FOpenLogicPinData SourcePinData = bInputPins
                ? NodeData.GetInputPinData(It.Key())
                : NodeData.GetOutputPinData(It.Key());
            FOpenLogicPinData TargetPinData = bInputPins
                ? TargetNode.GetOutputPinData(PinConnection.PinID)
                : TargetNode.GetInputPinData(PinConnection.PinID);

            if (SourcePinData.Role != TargetPinData.Role)
            {
                PinState.Connections.RemoveAt(i);
                bHasModifiedPins = true;
                continue;
            }

            if (SourcePinData.Role == EPinRole::DataProperty)
            {
                UOpenLogicProperty* SourceProperty = SourcePinData.PropertyClass.GetDefaultObject();
                UOpenLogicProperty* TargetProperty = TargetPinData.PropertyClass.GetDefaultObject();
                if (!SourceProperty->CanConnectTo(TargetProperty))
                {
                    PinState.Connections.RemoveAt(i);
                    bHasModifiedPins = true;
                    continue;
                }
            }
        }

        if (PinState.DefaultValue.IsEmpty() && PinState.Connections.IsEmpty() && !PinState.IsUserCreated)
        {
            It.RemoveCurrent();
            bHasModifiedPins = true;
        }
    }

    return bHasModifiedPins;
}

void UGraphEditorBase::SetNodeSnapDistance(float NewSnapDistance)
{
	NodeSnapDistance = NewSnapDistance;
}

float UGraphEditorBase::GetNodeSnapDistance()
{
    return NodeSnapDistance;
}

void UGraphEditorBase::ClearNodes()
{
    TArray<UNodeBase*> NodeWidgets;
    Nodes.GenerateValueArray(NodeWidgets);

    for (UNodeBase* Node : NodeWidgets)
    {
        Node->RemoveFromParent();
        Node->OnNodeUnregistered();
    }

    // Clear properties
    SelectedNodes.Empty();
    GridCells.Empty();
    Nodes.Empty();

    OnClearNodesTriggered();
}

void UGraphEditorBase::ClearGraph()
{
    ClearNodes();
	ClearAllConnections();
	ActiveGraph = nullptr;
}

void UGraphEditorBase::AddSelectedNode(UNodeBase* NodeToSelect)
{
    if (!NodeToSelect)
    {
        return;
    }

    if (SelectedNodes.Contains(NodeToSelect))
	{
		return;
	}
    
    // Store the node in the selected nodes array
    SelectedNodes.Add(NodeToSelect);

    // Set the selection start position
    NodeToSelect->SetSelectionStartPosition(NodeToSelect->GetNodePosition());

    // Broadcast the node selected event
    NodeToSelect->OnNodeSelected();

    // Broadcast the node select state changed event
    OnNodeSelectStateChanged.Broadcast(NodeToSelect, true);
}

void UGraphEditorBase::SetSelectedNodes(const TArray<UNodeBase*>& NewSelectedNodes)
{
    TArray<UNodeBase*> SelectedNodesCopy = SelectedNodes;
    for (UNodeBase* Node : SelectedNodesCopy)
	{
		if (!NewSelectedNodes.Contains(Node))
		{
			RemoveSelectedNode(Node);
		}
	}

    for (UNodeBase* Node : NewSelectedNodes)
	{
		AddSelectedNode(Node);
	}
}

void UGraphEditorBase::RemoveSelectedNode(UNodeBase* NodeToDeselect)
{
	if (!NodeToDeselect)
	{
		return;
	}

	if (!SelectedNodes.Contains(NodeToDeselect))
	{
		return;
	}

	// Remove the node from the selected nodes array
	SelectedNodes.Remove(NodeToDeselect);

	// Broadcast the node deselected event
	NodeToDeselect->OnNodeDeselected();

	// Broadcast the node select state changed event
	OnNodeSelectStateChanged.Broadcast(NodeToDeselect, false);
}

void UGraphEditorBase::SelectAllNodes()
{
    TArray<UNodeBase*> NodeWidgets = GetNodes();

	for (UNodeBase* Node : NodeWidgets)
	{
		AddSelectedNode(Node);
	}
}

void UGraphEditorBase::ClearSelectedNodes()
{
    TArray<UNodeBase*> SelectedNodesCopy = SelectedNodes;

    for (UNodeBase* Node : SelectedNodesCopy)
	{
		RemoveSelectedNode(Node);
	}
}

// Creates a node widget based on the Task Class.
UNodeBase* UGraphEditorBase::CreateNodeReference(TSubclassOf<UOpenLogicTask> TaskClass)
{
	if (!IsValid(TaskClass))
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[UGraphEditorBase::CreateNodeReference] Task class is invalid."))
		return nullptr;
	}

    UOpenLogicTask* DefaultTaskObject = TaskClass.GetDefaultObject();
    FTaskData TaskData = DefaultTaskObject->TaskData;

	TSoftClassPtr<UNodeBase> NodeWidgetSoftClass = TaskData.CustomNodeWidgetClass;
	if (NodeWidgetSoftClass.IsNull())
	{
		if (GraphStyleAsset)
		{
			NodeWidgetSoftClass = GraphStyleAsset->DefaultNodeClass;
		}
	}

    TSubclassOf<UNodeBase> NodeWidgetClass = NodeWidgetSoftClass.LoadSynchronous();
    if (!NodeWidgetClass)
	{
    	UE_LOG(OpenLogicLog, Error, TEXT("[UGraphEditorBase::CreateNodeReference] Node widget class is null."))
		return nullptr;
	}

    UNodeBase* NodeWidget = CreateWidget<UNodeBase>(this, NodeWidgetClass);
    NodeWidget->SetOwningGraphEditor(this);

    NodeWidget->TaskClass = TaskClass;
    NodeWidget->TaskObject = NewObject<UOpenLogicTask>(NodeWidget, TaskClass);
    NodeWidget->TaskData = NodeWidget->TaskObject->TaskData;

    return NodeWidget;
}

UNodeBase* UGraphEditorBase::CreateNode(TSubclassOf<UOpenLogicTask> TaskClass, FVector2D Position, bool SaveNode, FGuid LoadID)
{
    if (!ActiveGraph)
    {
        return nullptr;
    }

    UOpenLogicTask* DefaultTaskObject = TaskClass.GetDefaultObject();

    // Get the node task data
    FTaskData TaskData = DefaultTaskObject->TaskData;

    // Force load the node widget class
    TaskData.CustomNodeWidgetClass.LoadSynchronous();

    if (TaskData.Type == ENodeType::Event && !DefaultTaskObject->IsEventMultiImplementable && this->ActiveGraph->GraphData.Events.Contains(TaskClass))
    {
        return nullptr;
    }

    // Create the node widget
    UNodeBase* NodeWidget = NewNodeReference(TaskClass, SaveNode, LoadID);
    NodeWidget->OnNodeRegistered();

    // Set the node position
    NodeWidget->SetNodePosition(Position);
    NodeWidget->SaveNodePosition(Position);

    return NodeWidget;
}

void UGraphEditorBase::DeleteNode(UNodeBase* NodeToDelete)
{
    check(NodeToDelete);

    FGuid NodeID = NodeToDelete->NodeID;
    TSubclassOf<UOpenLogicTask> TaskClass = NodeToDelete->TaskClass;

    // Remove all the connections from the node
    NodeToDelete->BreakAllPinsConnections();

    // Remove the node from its grid cell
    RemoveNodeFromCell(NodeToDelete, GetCellPosition(NodeToDelete->GetNodePosition()));

    // Remove the node entry from the map
    ActiveGraph->GraphData.Nodes.Remove(NodeToDelete->NodeID);

    // Remove the event data if the node is an event
    if (NodeToDelete->TaskData.Type == ENodeType::Event && ActiveGraph->GraphData.Events.Contains(TaskClass))
    {
        ActiveGraph->GraphData.Events[TaskClass].NodeId.Remove(NodeID);

        // Checks if the task class has no more nodes
        if (ActiveGraph->GraphData.Events[TaskClass].NodeId.IsEmpty())
        {
            ActiveGraph->GraphData.Events.Remove(TaskClass);
        }
    }

    // Remove the node reference
    Nodes.Remove(NodeID);

    // Remove the node from selected nodes
    RemoveSelectedNode(NodeToDelete);

    // Remove the node from parent
    NodeToDelete->RemoveFromParent();

    // Broadcast the node unregistered event
    NodeToDelete->OnNodeUnregistered();
}

void UGraphEditorBase::DeleteNodes(const TArray<UNodeBase*>& NodesToDelete)
{
	for (UNodeBase* Node : NodesToDelete)
	{
		DeleteNode(Node);
	}
}

void UGraphEditorBase::BindRuntimeWorker(UOpenLogicRuntimeGraph* NewRuntimeWorker)
{
    if (!NewRuntimeWorker)
    {
		return;
	}

    if (BoundRuntimeWorker)
    {
		UnbindRuntimeWorker();
	}

	BoundRuntimeWorker = NewRuntimeWorker;

    // Bind the runtime worker event delegates
	BoundRuntimeWorker->OnNodeActivated.AddDynamic(this, &UGraphEditorBase::OnRuntimeTaskActivated);
	BoundRuntimeWorker->OnNodeCompleted.AddDynamic(this, &UGraphEditorBase::OnRuntimeTaskCompleted);
}

void UGraphEditorBase::UnbindRuntimeWorker()
{
    if (!BoundRuntimeWorker)
    {
		return;
	}

    BoundRuntimeWorker->OnNodeActivated.RemoveDynamic(this, &UGraphEditorBase::OnRuntimeTaskActivated);
	BoundRuntimeWorker->OnNodeCompleted.RemoveDynamic(this, &UGraphEditorBase::OnRuntimeTaskCompleted);
	BoundRuntimeWorker = nullptr;
}

void UGraphEditorBase::OnRuntimeTaskActivated(UOpenLogicTask* ActivatedRuntimeTask)
{
    UNodeBase* NodeWidget = GetNodeByGuid(ActivatedRuntimeTask->GetGuid());
    if (NodeWidget)
    {
        NodeWidget->RuntimeTaskObject = ActivatedRuntimeTask;

        // Broadcast the runtime task activation event
        NodeWidget->OnRuntimeTaskActivated.Broadcast(ActivatedRuntimeTask);
    }
}

void UGraphEditorBase::OnRuntimeTaskCompleted(UOpenLogicTask* CompletedRuntimeTask)
{
    UNodeBase* NodeWidget = GetNodeByGuid(CompletedRuntimeTask->GetGuid());
    if (NodeWidget)
    {
		NodeWidget->RuntimeTaskObject = nullptr;

		// Broadcast the runtime task completion event
		NodeWidget->OnRuntimeTaskCompleted.Broadcast(CompletedRuntimeTask);
	}
}

bool UGraphEditorBase::SetZoomLevel(float NewZoomLevel)
{
    if (!ScaleBox_Graph)
    {
		return false;
	}

    NewZoomLevel = FMath::Clamp(NewZoomLevel, MinZoomLevel, MaxZoomLevel);

    if (GetZoomLevel() == NewZoomLevel)
    {
        return false;
    }

	ScaleBox_Graph->SetUserSpecifiedScale(NewZoomLevel);

    if (GridDynamicMaterial)
    {
		GridDynamicMaterial->SetScalarParameterValue(FName("ZoomLevel"), NewZoomLevel);
	}

    float OldZoomLevel = ScaleBox_Graph->GetUserSpecifiedScale();
    ScaleBox_Graph->SetUserSpecifiedScale(NewZoomLevel);

    OnZoomLevelChanged.Broadcast(OldZoomLevel, NewZoomLevel);

    return true;
}

float UGraphEditorBase::GetZoomLevel() const
{
    if (!ScaleBox_Graph)
    {
		return 1.0f;
	}

    return ScaleBox_Graph->GetUserSpecifiedScale();
}

void UGraphEditorBase::SetGraphPosition(FVector2D NewPosition, bool UpdateGridPosition)
{
    if (!CanvasPanel_Nodes)
    {
        return;
    }

    FVector2D CurrentPosition = GetGraphPosition();

    CurrentGraphPosition = NewPosition;

    CanvasPanel_Nodes->SetRenderTranslation(NewPosition);

    bCanSaveGraphPosition = true;

    if (UpdateGridPosition)
    {
        // Update the grid position
        SetGridPosition((-NewPosition * UWidgetLayoutLibrary::GetViewportScale(this)) * GetZoomLevel());
    }

    OnGraphPositionChanged.Broadcast(CurrentPosition, NewPosition);
}

FVector2D UGraphEditorBase::GetGraphPosition() const
{
    return CurrentGraphPosition;
}

FVector2D UGraphEditorBase::AbsoluteToGraphPosition(FVector2D AbsolutePosition) const
{
    if (!CanvasPanel_Nodes)
    {
        return FVector2D::ZeroVector;
    }

    return CanvasPanel_Nodes->GetTickSpaceGeometry().AbsoluteToLocal(AbsolutePosition);
}

FVector2D UGraphEditorBase::GetMousePosition() const
{
    if (GetOwningPlayer())
    {
        return UWidgetLayoutLibrary::GetMousePositionOnPlatform();
    }
    else {
        return UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
    }
}

const FVector2D UGraphEditorBase::GetGraphMousePosition() const
{
    return CanvasPanel_Nodes->GetCachedGeometry().AbsoluteToLocal(GetMousePosition());
}

void UGraphEditorBase::Timer_SaveGraphPosition()
{
    if (bCanSaveGraphPosition)
    {
		SaveGraphPosition(GetGraphPosition());
		bCanSaveGraphPosition = false;
	}
}

void UGraphEditorBase::SetGridPosition(FVector2D NewPosition)
{
    if (!GridDynamicMaterial)
    {
        return;
    }

    FLinearColor GridPosition = FLinearColor();
    GridPosition.R = NewPosition.X;
    GridPosition.G = NewPosition.Y;

    GridDynamicMaterial->SetVectorParameterValue(FName("Position"), GridPosition);
}

FVector2D UGraphEditorBase::GetGridPosition() const
{
    if (!GridDynamicMaterial)
    {
		return FVector2D::ZeroVector;
	}

	FLinearColor GridPosition = FLinearColor();
	GridDynamicMaterial->GetVectorParameterValue(FName("Position"), GridPosition);

	return FVector2D(GridPosition.R, GridPosition.G);
}

void UGraphEditorBase::RefreshStyle()
{
    if (!GraphStyleAsset || !Image_Background || !Image_GraphGrid)
    {
		return;
	}

    Image_Background->SetBrushTintColor(GraphStyleAsset->ShowBackground ? GraphStyleAsset->BackgroundColor : FLinearColor::Transparent);
    SetNodeSnapDistance(GraphStyleAsset->GridSnapSize);

    if (SelectionBox)
    {
        SelectionBox->SetBrush(GraphStyleAsset->SelectionBoxBrush);
        SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (GridRenderer)
    {
        GridRenderer->SetVisibility(GraphStyleAsset->ShowNativeGrid ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }

    // Grid Brush
    if (!GraphStyleAsset->CustomGridBrush)
    {
        Image_GraphGrid->SetBrush(FSlateNoResource());
        return;
    }

    UMaterialInterface* GridMaterialInstance = Cast<UMaterialInterface>(GraphStyleAsset->CustomGridBrush);
    Image_GraphGrid->SetBrushFromMaterial(GridMaterialInstance);

    if (Image_GraphGrid->GetDynamicMaterial())
    {
        GridDynamicMaterial = Image_GraphGrid->GetDynamicMaterial();
    }
}

void UGraphEditorBase::AddNodeToCell(UNodeBase* NodeToAdd, FIntPoint CellPosition)
{
    if (!NodeToAdd)
    {
        return;
    }

    if (!GridCells.Contains(CellPosition))
    {
        GridCells.Add(CellPosition, FOpenLogicGridCell());
    }

    GridCells[CellPosition].CellNodes.Add(NodeToAdd);
}

void UGraphEditorBase::RemoveNodeFromCell(UNodeBase* NodeToRemove, FIntPoint CellPosition)
{
    if (!NodeToRemove)
	{
		return;
	}

	if (!GridCells.Contains(CellPosition))
	{
		return;
	}

	GridCells[CellPosition].CellNodes.Remove(NodeToRemove);

    if (GridCells[CellPosition].CellNodes.IsEmpty())
	{
		GridCells.Remove(CellPosition);
	}
}

FIntPoint UGraphEditorBase::GetCellPosition(FVector2D Position) const
{
    return FIntPoint(FMath::FloorToInt(Position.X / CellSize), FMath::FloorToInt(Position.Y / CellSize));
}

TArray<FIntPoint> UGraphEditorBase::GetAllActiveCells() const
{
    TArray<FIntPoint> ActiveCells;
	GridCells.GetKeys(ActiveCells);

	return ActiveCells;
}

int32 UGraphEditorBase::GetCellSize() const
{
    return CellSize;
}

const TSet<UNodeBase*> UGraphEditorBase::GetNodesInCell(FIntPoint CellPosition) const
{
    if (!GridCells.Contains(CellPosition))
	{
		return TSet<UNodeBase*>();
	}

    return GridCells[CellPosition].CellNodes;
}

TArray<FIntPoint> UGraphEditorBase::GetBoxIntersectingCells(FVector2D Start, FVector2D End, bool IncludeEmpty) const
{
    FIntPoint StartCell = GetCellPosition(Start);
	FIntPoint EndCell = GetCellPosition(End);

    if (StartCell.X > EndCell.X) Swap(StartCell.X, EndCell.X);
    if (StartCell.Y > EndCell.Y) Swap(StartCell.Y, EndCell.Y);

	TArray<FIntPoint> IntersectingCells;

	for (int32 X = StartCell.X; X <= EndCell.X; X++)
	{
		for (int32 Y = StartCell.Y; Y <= EndCell.Y; Y++)
		{
            FIntPoint CellPosition = FIntPoint(X, Y);

            if (GridCells.Contains(CellPosition) || IncludeEmpty)
            {
                IntersectingCells.Add(CellPosition);
            }
		}
	}

	return IntersectingCells;
}

TArray<FIntPoint> UGraphEditorBase::GetVisibleCells() const
{
	FVector2D ViewportPosition = GetGraphPosition() * -1;
	FVector2D ViewportSize = FVector2D(CanvasPanel_Nodes->GetCachedGeometry().GetLocalSize());
	
	return GetBoxIntersectingCells(ViewportPosition, ViewportPosition + ViewportSize, false);
}

void UGraphEditorBase::SetTasksToLoad(const TArray<TSoftClassPtr<UOpenLogicTask>>& NewTasks)
{
    TasksToLoad = NewTasks;
}

TArray<TSoftClassPtr<UOpenLogicTask>> UGraphEditorBase::GetTasksToLoad() const
{
    return TasksToLoad;
}

// Creates a node widget based on the Task Class and caches it.
UNodeBase* UGraphEditorBase::NewNodeReference(TSubclassOf<UOpenLogicTask> TaskClass, bool SaveNode, FGuid LoadID)
{
    if (!IsValid(ActiveGraph) || !IsValid(TaskClass))
    {
    	UE_LOG(OpenLogicLog, Error, TEXT("[UGraphEditorBase::NewNodeReference] Graph is invalid or Task class is invalid."))
		return nullptr;
	}

    UNodeBase* NodeWidget = CreateNodeReference(TaskClass);
    if (!NodeWidget)
    {
    	UE_LOG(OpenLogicLog, Error, TEXT("[UGraphEditorBase::NewNodeReference] Node widget is invalid."))
        return nullptr;
    }

    if (!SaveNode)
    {
        NodeWidget->NodeID = LoadID;

        // Store the node in the graph editor
        Nodes.Add(NodeWidget->NodeID, NodeWidget);

        OnNodeAdded.Broadcast(NodeWidget);
        return NodeWidget;
    }

    // Generate a new GUID for the node
    const FGuid NodeID = FGuid::NewGuid();
    NodeWidget->NodeID = NodeID;

    // Store the node in the graph editor
    Nodes.Add(NodeID, NodeWidget);

	// Store the node in the graph object
	FOpenLogicNode NodeStateData = FOpenLogicNode();
	NodeStateData.TaskClass = TaskClass;
	ActiveGraph->GraphData.Nodes.Add(NodeID, NodeStateData);

    // Store the node in the graph events (if it's an event)
    if (NodeWidget->TaskObject->TaskData.Type == ENodeType::Event)
    {
        bool bEventClassExists = ActiveGraph->GraphData.Events.Contains(TaskClass);

        FOpenLogicEventContainer EventContainer = bEventClassExists ? ActiveGraph->GraphData.Events[TaskClass] : FOpenLogicEventContainer();
        EventContainer.NodeId.Add(NodeID);

        // Apply the new event container to the graph object
        ActiveGraph->GraphData.Events.Add(TaskClass, EventContainer);
    }

#if WITH_EDITOR
    ActiveGraph->Modify();
#endif

    OnGraphSaved.Broadcast(ActiveGraph->GraphData);
    OnNodeAdded.Broadcast(NodeWidget);

    return NodeWidget;
}

void UGraphEditorBase::SetNodePalette(TSubclassOf<UNodePaletteBase> NewNodePaletteClass)
{
    if (!NewNodePaletteClass)
    {
        UE_LOG(OpenLogicLog, Error, TEXT("[UGraphEditorBase::SetNodePalette] NewNodePaletteClass is invalid."));
        return;
    }

    NodePaletteClass = NewNodePaletteClass;

    if (NodePaletteWidget)
    {
        NodePaletteWidget->RemoveFromParent();
    }

    NodePaletteWidget = CreateWidget<UNodePaletteBase>(this, NodePaletteClass.Get());
    NodePaletteWidget->SetGraphEditor(this);
	NodePaletteWidget->SetIsEditor(GIsEditor && !GWorld->IsPlayInEditor());
    NodePaletteWidget->SetVisibility(ESlateVisibility::Collapsed);
	
    OnNodePaletteChanged(NodePaletteWidget);
}