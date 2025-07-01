// Copyright 2024 - NegativeNameSeller

#include "Widgets/NodeBase.h"

#include "OpenLogicV2.h"
#include "Widgets/GraphEditorBase.h"
#include "Widgets/DisplayableWidgetBase.h"
#include "Utility/OpenLogicUtility.h"
#include "Components/CanvasPanelSlot.h"
#include "Classes/OpenLogicGraph.h"
#include "TimerManager.h"
#include "Misc/OutputDeviceNull.h"

#if WITH_EDITOR
#include "SourceCodeNavigation.h"
#include "Subsystems/AssetEditorSubsystem.h"
#endif

FVector2D UNodeBase::GetPinLocalPosition_Implementation(const UExecutionPinBase* Pin)
{
	return FVector2D();
}

FVector2D UNodeBase::ComputePinLocalPosition(const UExecutionPinBase* Pin, FVector2D StartOffset)
{
	if (!Pin) return FVector2D::ZeroVector;

	ForceLayoutPrepass();
	
	const FGeometry& PinGeometry = IsValid(Pin->InPinWidget) ? Pin->InPinWidget->GetTickSpaceGeometry() : Pin->GetTickSpaceGeometry();
	const FGeometry& NodeGeometry = GetTickSpaceGeometry();

	/*// Check if the function should use FGeometry or keep it layout-driven
	if (PinGeometry.GetLocalSize() != FVector2D::ZeroVector)
	{
		const FVector2D PinCenter = PinGeometry.GetLocalSize() * 0.5f;

		FSlateLayoutTransform PinToNodeTransform = Concatenate(
			PinGeometry.GetAccumulatedLayoutTransform(),
			NodeGeometry.GetAccumulatedLayoutTransform().Inverse()
		);

		FVector2D PinLocalToNode = PinToNodeTransform.TransformPoint(PinCenter);

		return PinLocalToNode;
	}*/

	// Layout-driven fallback
	const FVector2D NodeSize = GetDesiredSize();
	const float PinTopOffset = ComputePinTopOffset(Pin);
	const float PinHalfHeight = Pin->GetDesiredSize().Y * 0.5f;

	const float PinX = Pin->IsInputPin()
		? StartOffset.X
		: NodeSize.X - StartOffset.X;

	const FVector2D FallbackPosition = FVector2D(PinX, StartOffset.Y + PinTopOffset + PinHalfHeight);

	return FallbackPosition;
}

FVector2D UNodeBase::ComputePinLocalPosition_BehaviorTree(const UExecutionPinBase* Pin, FVector2D StartOffset, const float PinSpacing)
{
	if (!Pin) return FVector2D::ZeroVector;

	ForceLayoutPrepass();

	const FVector2D NodeSize = GetDesiredSize();
	const FVector2D PinSize = Pin->GetDesiredSize();
	const float X = ComputePinHorizontalOffset(Pin, PinSpacing, true);
	
	// Input pin
	if (Pin->IsInputPin())
	{
		return FVector2D(
			StartOffset.X + X,
			-StartOffset.Y
		);
	} else
	{
		return FVector2D(
			StartOffset.X + X,
			StartOffset.Y + NodeSize.Y
		);
	}
}

float UNodeBase::ComputePinHorizontalOffset(const UExecutionPinBase* Pin, const float PinSpacing, bool bCenterAlign) const
{
	if (!Pin) return 0.0f;

	const TMap<int32, UExecutionPinBase*>& PinMap = Pin->IsInputPin() ? InputPins : OutputPins;
	TArray<UExecutionPinBase*> PinList;
	PinMap.GenerateValueArray(PinList);

	if (PinList.Num() == 1 && PinList[0] == Pin)
	{
		float NodeWidth = GetDesiredSize().X;
		return NodeWidth * 0.5f;
	}

	float TotalWidth = 0.0f;
	for (UExecutionPinBase* P : PinList)
	{
		P->ForceLayoutPrepass();
		TotalWidth += P->GetDesiredSize().X;
	}

	float X_Offset = 0.0f;
	for (UExecutionPinBase* CurrentPin : PinList)
	{
		if (CurrentPin == Pin)
		{
			break;
		}
		CurrentPin->ForceLayoutPrepass();
		X_Offset += CurrentPin->GetDesiredSize().X;
		X_Offset += PinSpacing;
	}

	if (bCenterAlign)
	{
		const float NodeWidth = GetDesiredSize().X;
		X_Offset += (NodeWidth - TotalWidth) * 0.5f;
	}

	return X_Offset;
}

float UNodeBase::ComputePinTopOffset(const UExecutionPinBase* Pin)
{
	if (!Pin) return 0.0f;

	const TMap<int32, UExecutionPinBase*>& PinMap = Pin->IsInputPin() ? InputPins : OutputPins;

	TArray<UExecutionPinBase*> PinList;
	PinMap.GenerateValueArray(PinList);

	float Y_Offset = 0.0f;

	for (UExecutionPinBase* CurrentPin : PinList)
	{
		if (CurrentPin == Pin)
		{
			break;
		}

		CurrentPin->ForceLayoutPrepass();

		const float PinHeight = CurrentPin->GetDesiredSize().Y;
		Y_Offset += PinHeight;
	}

	return Y_Offset;
}

// Sets the graph editor that owns this node.
void UNodeBase::SetOwningGraphEditor(UGraphEditorBase* InGraphEditor)
{
	OwningGraphEditor = InGraphEditor;
}

// Returns the graph editor that owns this node.
UGraphEditorBase* UNodeBase::GetOwningGraphEditor() const
{
	return OwningGraphEditor;
}

void UNodeBase::SetNodePosition(const FVector2D Position)
{
	UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Slot);

	UGraphEditorBase* GraphEditor = GetOwningGraphEditor();

	if (!PanelSlot || !GraphEditor)
	{
		return;
	}

	FVector2D SnappedPosition = FVector2D(
		FMath::GridSnap(Position.X, GraphEditor->GetNodeSnapDistance()),
		FMath::GridSnap(Position.Y, GraphEditor->GetNodeSnapDistance())
	);

	// Set the new position of the node
	PanelSlot->SetPosition(SnappedPosition);

	// Recalculate the pin connections
	RecalculateConnections();

	// Update the node cells
	UpdateNodeCells();
}

FVector2D UNodeBase::GetNodePosition() const
{
	if (const UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		return CanvasPanelSlot->GetPosition();
	}

	return FVector2D::ZeroVector;
}

void UNodeBase::SetSelectionStartPosition(FVector2D Position)
{
	SelectionStartPosition = Position;
}

FVector2D UNodeBase::GetSelectionStartPosition() const
{
	return SelectionStartPosition;
}

void UNodeBase::UpdateNodeCells()
{
	FVector2D Position = GetNodePosition();

	if (GetDesiredSize().IsZero())
	{
		ForceLayoutPrepass();
	}

	TArray<FIntPoint> NewNodeCells = OwningGraphEditor->GetBoxIntersectingCells(Position, Position + GetDesiredSize(), true);

	for (FIntPoint CurrentCell : NodeCells)
	{
		if (NewNodeCells.Contains(CurrentCell))
		{
			continue;
		}

		OwningGraphEditor->RemoveNodeFromCell(this, CurrentCell);
	}

	for (FIntPoint Cell : NewNodeCells)
	{
		if (NodeCells.Contains(Cell))
		{
			continue;
		}

		OwningGraphEditor->AddNodeToCell(this, Cell);
	}

	NodeCells = NewNodeCells;
}

UExecutionPinBase* UNodeBase::AddInputPin(FOpenLogicPinData PinData, bool IsUserCreated)
{
	return this->AddPin(PinData, IsUserCreated, EOpenLogicPinDirection::Input);
}

UExecutionPinBase* UNodeBase::AddOutputPin(FOpenLogicPinData PinData, bool IsUserCreated)
{
	return this->AddPin(PinData, IsUserCreated, EOpenLogicPinDirection::Output);
}

UExecutionPinBase* UNodeBase::AddPin(FOpenLogicPinData PinData, bool IsUserCreated, EOpenLogicPinDirection PinDirection)
{
	if (!PinClass) { return nullptr; }

	int32 PinCount = PinDirection == EOpenLogicPinDirection::Input ? GetInputPinsCount() : GetOutputPinsCount();

	// Create the pin widget
	UExecutionPinBase* NewPin = CreateWidget<UExecutionPinBase>(this, PinClass);
	NewPin->SetOwningGraphEditor(OwningGraphEditor);
	NewPin->SetOwningNode(this);
	NewPin->SetPinID(PinCount + 1);
	NewPin->SetPinDirection(PinDirection);

	// Add the pin to the appropriate pins map
	auto& PinsMap = PinDirection == EOpenLogicPinDirection::Input ? InputPins : OutputPins;
	PinsMap.Add(NewPin->GetPinID(), NewPin);

	// Call the OnPinAdded. This event should be used to add the pin widget to a container widget (such as a UWrapBox).
	this->OnPinAdded(NewPin);

	// Clear the property class if the pin role is flow
	if (PinData.Role == EPinRole::FlowControl)
	{
		PinData.PropertyClass = nullptr;
	}

	NewPin->SetPinInfo(PinData);
	NewPin->ReceivePinData(PinData);
	NewPin->CreateDefaultValueWidget();

	// Pin saving logic
	auto& SavePinsMap = PinDirection == EOpenLogicPinDirection::Input ? GetNodeStateData().InputPins : GetNodeStateData().OutputPins;

	if (IsUserCreated && !SavePinsMap.Contains(NewPin->GetPinID()))
	{
		SavePinsMap.Add(NewPin->GetPinID(), FOpenLogicPinState(PinData, true));
		SaveNode();
	}
	
	return NewPin;
}

void UNodeBase::ResolvePinTypes(TSubclassOf<UOpenLogicProperty> PropertyClass)
{
	if (!PropertyClass) { return; }

	for (UExecutionPinBase* Pin : GetAllPins())
	{
		Pin->ResolvePin(PropertyClass);
	}
}

void UNodeBase::ResetPinTypes()
{
	const TArray<UExecutionPinBase*> Pins = GetAllPins();
	bool bHasActiveConnections = false;

	for (UExecutionPinBase* Pin : Pins)
	{
		if (Pin->HasAnyConnections())
		{
			bHasActiveConnections = true;
			break;
		}
	}

	if (!bHasActiveConnections)
	{
		for (UExecutionPinBase* Pin : Pins)
		{
			Pin->ResetPinType();
		}
	}
}

void UNodeBase::ScheduleConnectionUpdate()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UNodeBase::RecalculateConnections, GetWorld()->GetDeltaSeconds(), false);

	GetOwningGraphEditor()->RegisterTimer(TimerHandle);
}

void UNodeBase::SaveNode()
{
	check(OwningGraphEditor);
	check(OwningGraphEditor->GetGraph());
	check(TaskObject);

	UOpenLogicGraph* GraphObject = OwningGraphEditor->GetGraph();

	TaskObject->OnGraphNodeSaved(this);

	#if WITH_EDITOR
		GraphObject->Modify();
	#endif
}

void UNodeBase::SaveNodePosition(FVector2D Position)
{
	GetNodeStateData().Position = Position;
	SaveNode();
}

void UNodeBase::DeleteNode()
{
	OwningGraphEditor->DeleteNode(this);
}

/* Recalculate the positions and directions of the connections */
void UNodeBase::RecalculateConnections() const
{
	TArray<UExecutionPinBase*> Pins = this->GetAllPins();

	for (UExecutionPinBase* Pin : Pins)
	{
		for (UCustomConnection* Connection : Pin->LinkConnection)
		{
			Connection->OnRecalculateConnection();
		}
	}
}

/* Breaks all the connections of the specified pins */
void UNodeBase::BreakPinsConnections(TArray<UExecutionPinBase*> PinConnectionsToBreak)
{
	for (UExecutionPinBase* Pin : PinConnectionsToBreak)
	{
		if (!Pin->LinkConnection.IsEmpty())
		{
			Pin->Unlink();
		}
	}
}

void UNodeBase::BreakAllPinsConnections()
{
	TArray<UExecutionPinBase*> InputPinsArray;
	TArray<UExecutionPinBase*> OutputPinsArray;

	InputPins.GenerateValueArray(InputPinsArray);
	OutputPins.GenerateValueArray(OutputPinsArray);

	TArray<UExecutionPinBase*> Pins;
	Pins.Append(InputPinsArray);
	Pins.Append(OutputPinsArray);

	BreakPinsConnections(Pins);
}

void UNodeBase::OnNodeSelected_Implementation() {}
void UNodeBase::OnNodeDeselected_Implementation() {}

void UNodeBase::LoadPins()
{
	// Checks if the pins have already been loaded
	if (HasLoadedPins) { 
		return; 
	}

	// Add the input and output pins to the node
	for (const FOpenLogicPinData Pin : TaskData.InputPins)
	{
		AddInputPin(Pin, false);
	}

	for (const FOpenLogicPinData Pin : TaskData.OutputPins)
	{
		AddOutputPin(Pin, false);
	}

	// Handles dynamically added pins
	TArray<int32> InputPinsKeys;
	GetNodeStateData().InputPins.GenerateKeyArray(InputPinsKeys);

	int32 InputPinCount = GetInputPinsCount();
	for (int32 InputID : InputPinsKeys)
	{
		if (InputID > InputPinCount)
		{
			FOpenLogicPinState PinInfo = GetNodeStateData().InputPins[InputID];
			if (PinInfo.IsUserCreated)
			{
				AddInputPin(PinInfo.PinData, true);
			}
			else {
				GetNodeStateData().InputPins.Remove(InputID);
				SaveNode();
			}
		}
	}

	TArray<int32> OutputPinsKeys;
	GetNodeStateData().OutputPins.GenerateKeyArray(OutputPinsKeys);

	int32 OutputPinCount = GetOutputPinsCount();
	for (int32 OutputID : OutputPinsKeys)
	{
		if (OutputID > OutputPinCount)
		{
			FOpenLogicPinState PinInfo = GetNodeStateData().OutputPins[OutputID];
			if (PinInfo.IsUserCreated)
			{
				AddOutputPin(PinInfo.PinData, true);
			}
			else {
				GetNodeStateData().OutputPins.Remove(OutputID);
				SaveNode();
			}
		}
	}

	// Mark the pins as loaded
	HasLoadedPins = true;
}

void UNodeBase::LoadWidgets()
{
	for (TSubclassOf<UDisplayableWidgetBase> WidgetClass : TaskObject->DisplayableWidgets)
	{
		AddDisplayableWidget(WidgetClass);
	}
}

FOpenLogicNode& UNodeBase::GetNodeStateData() const
{
	if (!GetOwningGraphEditor() || !GetOwningGraphEditor()->GetGraph())
	{
		return EmptyNodeState;
	}

	if (!GetOwningGraphEditor()->GetGraph()->GraphData.Nodes.Contains(NodeID))
	{
		return EmptyNodeState;
	}
	
	return GetOwningGraphEditor()->GetGraph()->GraphData.Nodes[NodeID];
}

TArray<UExecutionPinBase*> UNodeBase::GetAllPins() const
{
	TArray<UExecutionPinBase*> InputPinsArray;
	TArray<UExecutionPinBase*> OutputPinsArray;

	InputPins.GenerateValueArray(InputPinsArray);
	OutputPins.GenerateValueArray(OutputPinsArray);

	TArray<UExecutionPinBase*> Pins;
	Pins.Append(InputPinsArray);
	Pins.Append(OutputPinsArray);

	return Pins;
}

UExecutionPinBase* UNodeBase::GetInputPinByName(const FName& PinName) const
{
	for (const auto& PinPair : InputPins)
	{
		if (PinPair.Value->GetPinInfo().PinName == PinName)
		{
			return PinPair.Value;
		}
	}

	return nullptr;
}

UExecutionPinBase* UNodeBase::GetOutputPinByName(const FName& PinName) const
{
	for (const auto& PinPair : OutputPins)
	{
		if (PinPair.Value->GetPinInfo().PinName == PinName)
		{
			return PinPair.Value;
		}
	}

	return nullptr;
}

void UNodeBase::AddDisplayableWidget(TSubclassOf<UDisplayableWidgetBase> WidgetClass)
{
	check(WidgetClass);

	UDisplayableWidgetBase* Widget = CreateWidget<UDisplayableWidgetBase>(this, WidgetClass);
	Widget->Node = this;

	OnDisplayableWidgetAdded(Widget);
}

void UNodeBase::SaveTaskProperties()
{
	for (const TPair<FGuid, FName>& Pair : TaskObject->GetBlueprintTaskProperties())
	{
		SaveTaskProperty(Pair.Value);
	}
}

void UNodeBase::SaveTaskProperty(FName PropertyName)
{
	// Check if task object is valid
	if (!IsValid(TaskObject))
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[UNodeBase::SaveTaskProperty] TaskObject is not valid"));
		return;
	}

	// Check if the property name is valid
	if (PropertyName.IsNone())
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[UNodeBase::SaveTaskProperty] PropertyName is not valid"));
		return;
	}

	// Retrieve the property from the task object
	FProperty* Prop = FindFProperty<FProperty>(TaskObject->GetClass(), PropertyName);

	// Check if the property is valid
	if (!Prop)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[UNodeBase::SaveTaskProperty] Property is not valid"));
		return;
	}

	FString Content;
	Prop->ExportText_Direct(Content, Prop->ContainerPtrToValuePtr<void>(TaskObject), TaskObject, nullptr, PPF_None);

	if (Content.IsEmpty())
	{
		return;
	}

	if (TaskObject->IsCppProperty(PropertyName))
	{
		GetNodeStateData().CppContent.Add(PropertyName, Content);
	} else
	{
		GetNodeStateData().BlueprintContent.Add(TaskObject->GetPropertyIdentifierByName(PropertyName, true), Content);
	}
}

void UNodeBase::LoadTaskProperties()
{
	TMap<FGuid, FString> TempBlueprintContent = GetNodeStateData().BlueprintContent;
	TMap<FName, FString> TempCppContent = GetNodeStateData().CppContent;

	bool bDataChanged = false;
	
	for (const auto& BlueprintPair : TempBlueprintContent)
	{
		if (!TaskObject->DoesPropertyIdentifierExist(BlueprintPair.Key))
		{
			if (!TaskObject->GetDeprecatedTaskProperties().Contains(BlueprintPair.Key))
			{
				GetNodeStateData().BlueprintContent.Remove(BlueprintPair.Key);
				bDataChanged = true;
			}
			
			continue;
		}

		LoadTaskProperty(TaskObject->GetPropertyNameByGUID(BlueprintPair.Key), BlueprintPair.Value);
	}

	for (const auto& CppPair : TempCppContent)
	{
		if (!LoadTaskProperty(CppPair.Key, CppPair.Value))
		{
			GetNodeStateData().CppContent.Remove(CppPair.Key);
			bDataChanged = true;
		}
	}

	if (bDataChanged)
	{
		SaveNode();
	}
}

bool UNodeBase::LoadTaskProperty(const FName& PropertyName, const FString& PropertyValue) const
{
	FProperty* Property = TaskObject->GetClass()->FindPropertyByName(PropertyName);

	if (!Property)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[UNodeBase::LoadTaskProperty] Property is not valid"));
		return false;
	}

	// Parse the property value
	FOutputDeviceNull ErrorText;
	const TCHAR* Result = Property->ImportText_Direct(*PropertyValue, Property->ContainerPtrToValuePtr<void>(TaskObject), TaskObject, PPF_None, &ErrorText);

	return Result != nullptr;
}

void UNodeBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	const FVector2D CachedSize = GetCachedGeometry().Size;
	const FVector2D NewSize = MyGeometry.Size;

	if (CachedSize != NewSize)
	{
		GetOwningGraphEditor()->OnNodeDesiredSizeChanged.Broadcast(this, NewSize);
	}

	Super::NativeTick(MyGeometry, InDeltaTime);
}

FReply UNodeBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	MouseDownPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Unhandled();
	}

	FReply ReplyHandled = FReply::Handled();

	bool IsCtrlDown = InMouseEvent.IsControlDown();
	bool IsShiftDown = InMouseEvent.IsShiftDown();
	bool IsNodeSelected = OwningGraphEditor->IsNodeSelected(this);

	if (!IsCtrlDown && !IsShiftDown && !IsNodeSelected)
	{
		OwningGraphEditor->ClearSelectedNodes();
	}

	if (IsCtrlDown && IsNodeSelected)
	{
		OwningGraphEditor->RemoveSelectedNode(this);
		return ReplyHandled;
	}

	OwningGraphEditor->AddSelectedNode(this);
	OwningGraphEditor->SetOffsetOriginNode(this);

	return ReplyHandled;
}

FReply UNodeBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	MouseDownPosition = FVector2D::ZeroVector;

	FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (Reply.IsEventHandled())
	{
		return Reply;
	}

	if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Unhandled();
	}

	OwningGraphEditor->SetOffsetOriginNode(nullptr);

	return FReply::Handled();
}

FReply UNodeBase::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);

	if (Reply.IsEventHandled() || !GetOwningGraphEditor()->IsEditorGraph())
	{
		return Reply;
	}

#if WITH_EDITOR
	// Open the blueprint editor if the task class is a blueprint
	UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(TaskClass);
	if (BlueprintClass && BlueprintClass->ClassGeneratedBy)
	{
		UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintClass->ClassGeneratedBy);

		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
		
	} else
	{
		// Open the source code file if the task class is a C++ class
		FString ClassHeaderPath;
		if (FSourceCodeNavigation::FindClassHeaderPath(TaskClass, ClassHeaderPath) && !ClassHeaderPath.IsEmpty())
		{
			FSourceCodeNavigation::OpenSourceFile(ClassHeaderPath);
		}
	}
#endif
	
	return FReply::Unhandled();
}

FTaskData UNodeBase::InitializeNode(UTextBlock* NameTextBlock, UTextBlock* TypeTextBlock)
{
	if (!IsValid(TaskClass))
	{
		return FTaskData();
	}

	if (IsValid(NameTextBlock))
	{
		NameTextBlock->SetText(FText::FromString(TaskData.Name));
		NameTextBlock->SetToolTipText(TaskData.Description);
	}

	if (IsValid(TypeTextBlock))
	{
		if (TaskData.Type == ENodeType::Function)
		{
			// If node type is a function then set text block Text to 'Function'
			TypeTextBlock->SetText(FText::FromString("Function"));
		}
		else {
			// If node type is an event then set text block Text to 'Event'
			TypeTextBlock->SetText(FText::FromString("Event"));
		}
	}

	LoadPins();
	LoadTaskProperties();
	LoadWidgets();

	TaskObject->OnGraphNodeInitialized(this);

	return TaskData;
}
