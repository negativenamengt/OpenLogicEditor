// Copyright 2024 - NegativeNameSeller

#include "Classes/CustomConnection.h"
#include "Widgets/ExecutionPinBase.h"
#include "Widgets/NodeBase.h"
#include "Classes/OpenLogicGraph.h"

void UCustomConnection::OnConnectionInitialized_Implementation()
{}

void UCustomConnection::OnRecalculateConnection_Implementation()
{}

void UCustomConnection::OnPaint_Implementation(UPARAM(ref)FPaintContext& Context)
{}

void UCustomConnection::ConnectPins(UExecutionPinBase* InSourcePin, UExecutionPinBase* InTargetPin, bool IsPreviewConnection, bool ShouldSaveConnection)
{
	SourcePin = InSourcePin;
	TargetPin = InTargetPin;
	bIsPreviewConnection = IsPreviewConnection;
	
	if (bIsPreviewConnection)
	{
		return;
	}

	// Store the connection in the pins
	SourcePin->LinkConnection.AddUnique(this);
	TargetPin->LinkConnection.AddUnique(this);

	// Notify the pins that a connection has been made
	SourcePin->OnConnected(TargetPin, this);
	TargetPin->OnConnected(SourcePin, this);

	// Hide the pin default value widget if it exists
	if (TargetPin->GetDefaultValueWidget())
	{
		TargetPin->GetDefaultValueWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (!ShouldSaveConnection)
	{
		return;
	}

	// Add the connection to the graph data.
	UpdateGraphDataConnection(true);
}

void UCustomConnection::DisconnectPins()
{
	if (!SourcePin || !TargetPin)
	{
		return;
	}

	// Remove the connection from the pins
	SourcePin->LinkConnection.Remove(this);
	TargetPin->LinkConnection.Remove(this);

	// Notify the pins that the connection has been removed
	SourcePin->OnUnlink();
	TargetPin->OnUnlink();

	// Show the pin default value widget if it exists
	if (TargetPin->GetDefaultValueWidget() && !TargetPin->HasAnyConnections())
	{
		TargetPin->GetDefaultValueWidget()->SetVisibility(ESlateVisibility::Visible);
	}

	// Remove the connection from the graph data.
	UpdateGraphDataConnection(false);
}

void UCustomConnection::UpdateGraphDataConnection(bool bAddConnection)
{
	UNodeBase* SourceNode = SourcePin->GetOwningNode();
	UNodeBase* TargetNode = TargetPin->GetOwningNode();

	FOpenLogicPinConnection SourceConnectionData = FOpenLogicPinConnection(SourceNode->GetNodeID(), SourcePin->GetPinID());
	FOpenLogicPinConnection TargetConnectionData = FOpenLogicPinConnection(TargetNode->GetNodeID(), TargetPin->GetPinID());

	UpdatePinConnectionData(SourceNode->GetNodeStateData().OutputPins, SourcePin->GetPinID(), TargetConnectionData, bAddConnection);
	UpdatePinConnectionData(TargetNode->GetNodeStateData().InputPins, TargetPin->GetPinID(), SourceConnectionData, bAddConnection);

	SourceNode->SaveNode();
	TargetNode->SaveNode();
}

void UCustomConnection::UpdatePinConnectionData(TMap<int32, FOpenLogicPinState>& NodePins, int32 PinID, const FOpenLogicPinConnection& ConnectionData, bool bAddConnection)
{
	// Check if the pin entry exists in the node data
	if (!NodePins.Contains(PinID))
	{
		NodePins.Add(PinID, FOpenLogicPinState());
	}

	if (bAddConnection)
	{
		NodePins[PinID].Connections.Add(ConnectionData);
	} else
	{
		NodePins[PinID].Connections.Remove(ConnectionData);

		if (NodePins[PinID].Connections.Num() == 0)
		{
			NodePins.Remove(PinID);
		}
	}
}

UWorld* UCustomConnection::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject)) { return nullptr; }
	else { return GetOuter()->GetWorld(); }
}
