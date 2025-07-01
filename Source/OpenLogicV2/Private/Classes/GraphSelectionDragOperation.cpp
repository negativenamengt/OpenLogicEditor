// Copyright 2024 - NegativeNameSeller

#include "Classes/GraphSelectionDragOperation.h"

void UGraphSelectionDragOperation::Initialize(UGraphEditorBase* InOwningGraphEditor)
{
	if (!InOwningGraphEditor)
	{
		return;
	}

	OwningGraphEditor = InOwningGraphEditor;

	// Set the selection box to be visible
	OwningGraphEditor->SelectionBox->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Set the selection box to the initial position
	UCanvasPanelSlot* SelectionBoxSlot = GetSelectionBoxSlot();

	if (SelectionBoxSlot)
	{
		SelectionBoxSlot->SetPosition(OwningGraphEditor->GetGraphMousePosition());
	}
}

void UGraphSelectionDragOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	if (!OwningGraphEditor)
	{
		return;
	}

	// Get the box position
	FVector2D BoxPosition = GetSelectionBoxSlot()->GetPosition();

	// Get the new box size
	FVector2D BoxSize = OwningGraphEditor->GetGraphMousePosition() - BoxPosition;

	// Set the size to the absolute value
	GetSelectionBoxSlot()->SetSize(BoxSize.GetAbs());

	// Determine the scale to apply
	FVector2D RenderScale(1.0f, 1.0f);
	if (BoxSize.X < 0.0f)
	{
		BoxPosition.X += BoxSize.X;  // Adjust position to the left
		RenderScale.X = -1.0f;
	}
	if (BoxSize.Y < 0.0f)
	{
		BoxPosition.Y += BoxSize.Y;  // Adjust position upward
		RenderScale.Y = -1.0f;
	}

	// Set the render transform scale
	OwningGraphEditor->SelectionBox->SetRenderScale(RenderScale);
	
	// Update the BoxSize to be positive
	BoxSize = BoxSize.GetAbs();

	// Get the grid cells that are intersected by the selection box
	TArray<FIntPoint> IntersectedCells = OwningGraphEditor->GetBoxIntersectingCells(BoxPosition, BoxSize + BoxPosition, false);

	TArray<UNodeBase*> SelectedNodes;

	// Iterate over the intersected cells
	for (const FIntPoint& Cell : IntersectedCells)
	{
		for (UNodeBase* CellNode : OwningGraphEditor->GetNodesInCell(Cell))
		{
			// Check if the node is within the selection box
			FVector2D NodePosition = CellNode->GetNodePosition();
			FVector2D NodeDesiredSize = CellNode->GetDesiredSize();

			// Check with the most efficient (performance-wise) method to see if the node is within the selection box
			if (BoxPosition.X < NodePosition.X + NodeDesiredSize.X && BoxPosition.X + BoxSize.X > NodePosition.X &&
				BoxPosition.Y < NodePosition.Y + NodeDesiredSize.Y && BoxPosition.Y + BoxSize.Y > NodePosition.Y)
			{
				SelectedNodes.Add(CellNode);
			}
		}
	}

	// Update the selection
	OwningGraphEditor->SetSelectedNodes(SelectedNodes);
}

void UGraphSelectionDragOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	ClearSelectionBox();
}

void UGraphSelectionDragOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	ClearSelectionBox();
}

void UGraphSelectionDragOperation::ClearSelectionBox()
{
	if (!OwningGraphEditor)
	{
		return;
	}

	OwningGraphEditor->SelectionBox->SetVisibility(ESlateVisibility::Collapsed);
}

UCanvasPanelSlot* UGraphSelectionDragOperation::GetSelectionBoxSlot()
{
	if (!OwningGraphEditor)
	{
		return nullptr;
	}

	UImage* SelectionBox = OwningGraphEditor->SelectionBox;

	return Cast<UCanvasPanelSlot>(SelectionBox->Slot);
}
