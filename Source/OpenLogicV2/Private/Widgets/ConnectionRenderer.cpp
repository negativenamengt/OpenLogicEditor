// Copyright 2024 - NegativeNameSeller

#include "Widgets/ConnectionRenderer.h"
#include "Widgets/GraphEditorBase.h"

int32 UConnectionRenderer::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (!IsValid(OwningGraphEditor))
	{
		return LayerId;
	}

    FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    for (UCustomConnection* It : OwningGraphEditor->GetActiveConnections())
    {
        It->OnPaint(Context);
    }

    if (!bDisplayDebugCells)
    {
        return LayerId;
    }

    // Render debug grid cells
    TArray<FIntPoint> GridCells = OwningGraphEditor->GetAllActiveCells();
    for (int i = 0; i < GridCells.Num(); i++)
    {
        int32 CellSize = OwningGraphEditor->GetCellSize();

        FVector2D Start = FVector2D(GridCells[i].X * CellSize, GridCells[i].Y * CellSize);
        FVector2D End = Start + FVector2D(CellSize, CellSize);

        FSlateDrawElement::MakeLines(
			Context.OutDrawElements,
			Context.LayerId,
			Context.AllottedGeometry.ToPaintGeometry(),
			{ Start, FVector2D(End.X, Start.Y), End, FVector2D(Start.X, End.Y), Start },
			ESlateDrawEffect::None,
			FLinearColor::Red,
			true,
			1.0f
		);
    }

    return LayerId;
}