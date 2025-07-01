// Copyright 2024 - NegativeNameSeller

#include "Widgets/ExecutionPinBase.h"

#include "Blueprint/WidgetTree.h"
#include "Components/NativeWidgetHost.h"
#include "Widgets/ConnectionBase.h"
#include "Widgets/GraphEditorBase.h"

const FText UExecutionPinBase::GetFriendlyPinToolTipText() const
{
	// Get the pin name and description
	const FString PinName = PinInfo.PinName.ToString();
	const FText PinDescription = PinInfo.Description;

	// Determine the pin type
	FString PinType = GetPropertyDefaultObject() ? GetPropertyDefaultObject()->PropertyDisplayName.ToString() : "Exec";

	// Construct the tooltip text
	FString PinToolTipText = FString::Printf(TEXT("%s\n%s"), *PinName, *PinType);

	// Add the description if it's not empty
	if (!PinDescription.IsEmpty())
	{
		PinToolTipText += FString::Printf(TEXT("\n\n%s"), *PinDescription.ToString());
	}

	return FText::FromString(PinToolTipText);
}

TSubclassOf<UOpenLogicProperty> UExecutionPinBase::GetPropertyClass() const
{
	if (ResolvedPropertyClass)
	{
		return ResolvedPropertyClass;
	}
	
	return GetPinInfo().PropertyClass;
}

UOpenLogicProperty* UExecutionPinBase::GetPropertyDefaultObject() const
{
	if (!GetPinInfo().PropertyClass)
	{
		return nullptr;
	}

	if (ResolvedPropertyClass)
	{
		return ResolvedPropertyClass.GetDefaultObject();
	}

	return GetPinInfo().PropertyClass.GetDefaultObject();
}

FLinearColor UExecutionPinBase::GetPinColor(FLinearColor FallbackColor) const
{
	UOpenLogicProperty* Property = GetPropertyDefaultObject();
	if (IsFlowControlPin() || !Property)
	{
		return FallbackColor;
	}

	return Property->PropertyColor;
}

FText UExecutionPinBase::GetPinDisplayName() const
{
	const FName PinName = PinInfo.PinName;
	
	if (PinName.IsNone())
	{
		return FText::GetEmpty();
	}

	if (PinName == "execute" || PinName == "then")
	{
		return FText::GetEmpty();
	}

	return FText::FromName(PinName);
}

EPinRole UExecutionPinBase::GetPinRole() const
{
	return PinInfo.Role;
}

void UExecutionPinBase::BeginConnectionPreview()
{
	if (OwningGraphEditor->IsPreviewConnectionActive())
	{
		UExecutionPinBase* Other = OwningGraphEditor->GetCurrentPreviewConnection()->GetSourcePin();

		OwningGraphEditor->DestroyConnectionPreview();
		OwningGraphEditor->CreateConnection(Other, this, false, true);
		return;
	}

	// Creating the preview connection
	OwningGraphEditor->CreateConnection(this, nullptr, true);

	this->OnConnectionPreviewStarted();
}

void UExecutionPinBase::ReceivePinData_Implementation(FOpenLogicPinData PinInformation)
{
}

void UExecutionPinBase::ReceiveDefaultValueWidget_Implementation(const FOpenLogicPinValueDisplay& DisplayData)
{
}

EPinConnectionValidity UExecutionPinBase::CanConnectTo(UExecutionPinBase* Other) const
{
	if (!Other)
	{
		return EPinConnectionValidity::None;
	}

	// Checks if the same pin is being connected to itself
	if (this == Other)
	{
		return EPinConnectionValidity::IdenticalPin;
	}

	// Checks if the pin is being connected to a pin of the same node
	if (GetOwningNode() == Other->GetOwningNode())
	{
		return EPinConnectionValidity::IdenticalNode;
	}

	if (this->IsInputPin() == Other->IsInputPin())
	{
		return EPinConnectionValidity::DirectionConflict;
	}

	// Checks if the pin is being connected to a pin with a different role
	if (PinInfo.Role != Other->PinInfo.Role)
	{
		return EPinConnectionValidity::RoleMismatch;
	}

	// Checks if the property object can connect to the other pin's property object
	if (PinInfo.Role == EPinRole::DataProperty && !GetPropertyDefaultObject()->CanConnectTo(Other->GetPropertyDefaultObject()))
	{
		return EPinConnectionValidity::PropertyMismatch;
	}

	if (IsReplaceable())
	{
		return EPinConnectionValidity::Replaceable;
	}

	if (Other->IsReplaceable())
	{
		return EPinConnectionValidity::Replaceable;
	}

	return EPinConnectionValidity::Compatible;
}

const bool UExecutionPinBase::IsReplaceable() const
{
	if (IsInputPin() && !LinkConnection.IsEmpty() && PinInfo.Role == EPinRole::DataProperty)
	{
		return true;
	}

	if (IsOutputPin() && !LinkConnection.IsEmpty() && (PinInfo.Role == EPinRole::FlowControl || GetPropertyDefaultObject() && !GetPropertyDefaultObject()->CanConnectToMultiple))
	{
		return true;
	}

	return false;
}

FVector2D UExecutionPinBase::GetGraphPosition() const
{
	if (!OwningNode)
	{
		return FVector2D::ZeroVector;
	}

	return OwningNode->GetPinLocalPosition(this) + OwningNode->GetNodePosition();
}

void UExecutionPinBase::CreateDefaultValueWidget()
{
	if (IsOutputPin() || PinInfo.Role != EPinRole::DataProperty || !GetPropertyDefaultObject())
	{
		return;
	}

	if (!bShouldCreateDefaultValueWidget)
	{
		return;
	}

	TSharedPtr<FOpenLogicDefaultValueHandle> Handle = MakeShared<FOpenLogicDefaultValueHandle>();
	Handle->SetExecutionPin(this);
	
	FOpenLogicPinValueDisplay WidgetData = GetPropertyDefaultObject()->CreatePinDefaultValueWidget();
	if (!WidgetData.Widget)
	{
		TArray<TSharedRef<SWidget>> SlateWidgets = GetPropertyDefaultObject()->CreateEditorInputWidgets(Handle.ToSharedRef(), PinInfo.Attributes);
		if (SlateWidgets.IsEmpty() || SlateWidgets.Num() > 1)
		{
			return;
		}

		UNativeWidgetHost* NativeWidgetHost = NewObject<UNativeWidgetHost>();
		NativeWidgetHost->SetContent(SlateWidgets[0]);
		WidgetData.Widget = NativeWidgetHost;
	}

	DefaultValueWidget = WidgetData.Widget;
	ReceiveDefaultValueWidget(WidgetData);
}

void UExecutionPinBase::DestroyDefaultValueWidget()
{
	if (!DefaultValueWidget)
	{
		return;
	}

	DefaultValueWidget->RemoveFromParent();
	DefaultValueWidget = nullptr;
}

void UExecutionPinBase::SetDefaultValue(FOpenLogicDefaultValue NewDefaultValue)
{
	if (!GetPropertyDefaultObject())
	{
		return;
	}

	TMap<int32, FOpenLogicPinState>& PinMap = this->IsInputPin()
		? GetOwningNode()->GetNodeStateData().InputPins
		: GetOwningNode()->GetNodeStateData().OutputPins;

	FOpenLogicPinState& PinState = PinMap.FindOrAdd(PinID);

	// Check if the pin data default value is the same as the new default value
	if (PinInfo.DefaultValue == NewDefaultValue)
	{
		PinState.DefaultValue.Clear();
	} else
	{
		PinState.DefaultValue = NewDefaultValue;	
	}

	UOpenLogicGraph* Graph = GetOwningGraphEditor()->GetGraph();
	Graph->Modify();
	GetOwningGraphEditor()->OnGraphSaved.Broadcast(Graph->GraphData);
}

FOpenLogicDefaultValue UExecutionPinBase::GetDefaultValue() const
{
	if (!GetPropertyDefaultObject())
	{
		return FOpenLogicDefaultValue();
	}

	TMap<int32, FOpenLogicPinState>& PinMap = this->IsInputPin()
	? GetOwningNode()->GetNodeStateData().InputPins
	: GetOwningNode()->GetNodeStateData().OutputPins;

	if (!PinMap.Contains(PinID) || PinMap[PinID].DefaultValue.IsEmpty())
	{
		return PinInfo.DefaultValue;
	}
	
	FOpenLogicPinState& PinState = PinMap.FindOrAdd(PinID);

	// Create a copy of the default value
	return PinState.DefaultValue;
}

void UExecutionPinBase::ResolvePin(TSubclassOf<UOpenLogicProperty> PropertyClass)
{
	if (!PropertyClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Deresolve pin"));
		return;
	}

	if (GetPropertyDefaultObject() && GetPropertyDefaultObject()->bResolvesTypeDynamically)
	{
		ResolvedPropertyClass = PropertyClass;
		SetForegroundColor(GetPinColor());
		CreateDefaultValueWidget();
	}
}

void UExecutionPinBase::ResetPinType()
{
	if (ResolvedPropertyClass)
	{
		ResolvedPropertyClass = nullptr;
		SetForegroundColor(GetPinColor());
		DestroyDefaultValueWidget();
	}
}

void UExecutionPinBase::TryRemovePin()
{
	TMap<int32, FOpenLogicPinState> SavedPinMap = this->IsInputPin()
		? GetOwningNode()->GetNodeStateData().InputPins
		: GetOwningNode()->GetNodeStateData().OutputPins;

	if (!SavedPinMap.Contains(PinID))
	{
		return;
	}

	if (SavedPinMap[PinID].IsUserCreated)
	{
		Internal_RemovePin(this->IsInputPin() ? GetOwningNode()->InputPins : GetOwningNode()->OutputPins, SavedPinMap);
	}
}

void UExecutionPinBase::Internal_RemovePin(TMap<int32, UExecutionPinBase*>& PinMap, TMap<int32, FOpenLogicPinState>& SavePinMap)
{
	// Remove all the connections from this pin
	this->Unlink();

	// Remove the pin reference
	PinMap.Remove(PinID);
	SavePinMap.Remove(PinID);

	TArray<UExecutionPinBase*> PinArray;
	PinMap.GenerateValueArray(PinArray);

	for (UExecutionPinBase* Pin : PinArray)
	{
		if (Pin->PinID < PinID)
		{
			continue;
		}

		int NewPinID = Pin->PinID - 1;

		// Assign the new pin ID
		Pin->PinID = NewPinID;

		FOpenLogicPinState NewPinData;

		// Update the pin ID in the saved pin map
		SavePinMap.RemoveAndCopyValue(Pin->PinID, NewPinData);
		SavePinMap.Add(NewPinID, NewPinData);

		// Update the connections pin ID
		for (const FOpenLogicPinConnection& Connection : SavePinMap[NewPinID].Connections)
		{
			UNodeBase* OtherNode = OwningGraphEditor->GetNodeByGuid(Connection.NodeID);
			if (!OtherNode)
			{
				continue;
			}

			TArray<FOpenLogicPinConnection>& OtherConnections = OtherNode->GetNodeStateData().InputPins[Connection.PinID].Connections;

			for (FOpenLogicPinConnection& OtherConnection : OtherConnections)
			{
				if (OtherConnection.NodeID == GetOwningNode()->GetNodeID())
				{
					OtherConnection.PinID = NewPinID;
				}
			}

			// Update the other node data
			OtherNode->SaveNode();
		}
	}

	// Update the node data
	GetOwningNode()->SaveNode();
	
	// Remove the pin from its parent widget
	this->RemoveFromParent();
}

void UExecutionPinBase::Unlink()
{
	if (LinkConnection.IsEmpty()) {
		return;
	}

	const TArray<UCustomConnection*> LinkConnectionArray = this->GetConnections();

	for (UCustomConnection* Connection : LinkConnectionArray)
	{
		OwningGraphEditor->RemoveConnection(Connection);

		// Mark the connection for deletion
		Connection->MarkAsGarbage();
	}

	LinkConnection.Empty();
}

void UExecutionPinBase::OnConnected(UExecutionPinBase* Other, UCustomConnection* ConnectionWidget)
{
	ReceiveOnConnected(Other, ConnectionWidget);

	if (GetDefaultValueWidget())
	{
		GetDefaultValueWidget()->SetVisibility(ESlateVisibility::Hidden);
	}

	UOpenLogicProperty* Property = GetPropertyDefaultObject();
	if (Property && Property->bResolvesTypeDynamically && Other->GetPropertyDefaultObject() != Property)
	{
		GetOwningNode()->ResolvePinTypes(Other->GetPropertyClass());
	}
}

void UExecutionPinBase::OnUnlink()
{
	ReceiveOnUnlink();

	if (GetDefaultValueWidget() && !HasAnyConnections())
	{
		GetDefaultValueWidget()->SetVisibility(ESlateVisibility::Visible);
	}

	if (ResolvedPropertyClass && !HasAnyConnections())
	{
		GetOwningNode()->ResetPinTypes();
	}
}