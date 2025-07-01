// Copyright 2024 - NegativeNameSeller

#include "Classes/DefaultNodeConnection.h"
#include "Widgets/ExecutionPinBase.h"
#include "Widgets/GraphEditorBase.h"
#include "Components/CanvasPanel.h"

#if WITH_EDITOR

bool UDefaultNodeConnection::CanEditChange(const FProperty* InProperty) const
{
    const bool bIsEditable = Super::CanEditChange(InProperty);

    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UDefaultNodeConnection, TangentScaleFactor))
	{
        return bIsEditable && DrawingStyle == EWireDrawingStyle::Spline;
	}

    return bIsEditable;
}

#endif

void UDefaultNodeConnection::RecacheValues()
{
    CachedWireData.SourcePosition = GetSourcePin()->GetGraphPosition() + GetSourcePinPositionOffset();
    CachedWireData.TargetPosition = GetTargetPin()->GetGraphPosition() + GetSourcePinPositionOffset();

    CacheInterpolation();
}

void UDefaultNodeConnection::RecachePreviewValues()
{
    if (!GetIsPreviewConnection())
	{
		return;
	}

    CachedWireData.SourcePosition = GetSourcePin()->GetGraphPosition() + GetSourcePinPositionOffset();

    UNodePaletteBase* NodePalette = GetOwningGraphEditor()->GetNodePaletteWidget();
    FVector2D MousePosition = GetOwningGraphEditor()->GetGraphMousePosition();

    CachedWireData.TargetPosition = NodePalette->IsVisible() ? CachedWireData.TargetPosition : MousePosition;

    CacheInterpolation();
}

void UDefaultNodeConnection::OnPaint_Implementation(UPARAM(ref)FPaintContext& Context)
{
    if (GetIsPreviewConnection())
    {
        RecachePreviewValues();
    }

    switch (DrawingStyle)
    {
        case EWireDrawingStyle::Spline:
            DrawSpline(Context);
            break;

        case EWireDrawingStyle::Subway:
            DrawInterpolatedPoints(Context);
            break;
    }
}

void UDefaultNodeConnection::OnRecalculateConnection_Implementation()
{
    if (!GetIsPreviewConnection())
    {
		RecacheValues();
	}
}

void UDefaultNodeConnection::DrawSpline(UPARAM(ref)FPaintContext& Context)
{
    FSlateDrawElement::MakeSpline(
        Context.OutDrawElements,
        Context.MaxLayer,
        Context.AllottedGeometry.ToPaintGeometry(),
        CachedWireData.SourcePosition,
        CachedWireData.DirectionalTangent,
        CachedWireData.TargetPosition,
        CachedWireData.DirectionalTangent,
        Thickness,
        ESlateDrawEffect::None,
        GetSourcePin()->GetPinColor()
    );
}

void UDefaultNodeConnection::DrawInterpolatedPoints(UPARAM(ref)FPaintContext& Context)
{
	FSlateDrawElement::MakeLines(
		Context.OutDrawElements,
		Context.MaxLayer,
		Context.AllottedGeometry.ToPaintGeometry(),
        CachedWireData.Points,
		ESlateDrawEffect::None,
		GetSourcePin()->GetPinColor(),
		true,
		Thickness
	);
}

void UDefaultNodeConnection::CacheInterpolation()
{
    switch (DrawingStyle)
    {
        case EWireDrawingStyle::Spline:
            CachedWireData.DirectionalTangent = CalculateDirectionalTangent();
			break;

        case EWireDrawingStyle::Subway:
            CachedWireData.Points = CalculateSubwayPoints();
            break;
    }
}

float UDefaultNodeConnection::GetPinOffset()
{
    check(GetSourcePin());
    return GetSourcePin()->IsInputPin() ? -PinOffset : PinOffset;
}

const bool UDefaultNodeConnection::IsDrawingHorizontal() const
{
    return DrawingOrientation == EOrientation::Orient_Horizontal;
}

const bool UDefaultNodeConnection::IsDrawingVertical() const
{
    return DrawingOrientation == EOrientation::Orient_Vertical;
}

FVector2D UDefaultNodeConnection::GetSourcePinPositionOffset()
{
    return GetSourcePin()->IsInputPin() ? -PositionOffset : PositionOffset;
}

FVector2D UDefaultNodeConnection::GetTargetPinPositionOffset()
{
    return GetTargetPin()->IsInputPin() ? -PositionOffset : PositionOffset;
}

FVector2D UDefaultNodeConnection::CalculateDirectionalTangent()
{
    float Result = this->IsDrawingVertical()
        ? FMath::Abs(CachedWireData.SourcePosition.Y - CachedWireData.TargetPosition.Y)
		: FVector2D::Distance(CachedWireData.SourcePosition, CachedWireData.TargetPosition);

    Result = (Result / 100.0f) * TangentScaleFactor;

    if (GetSourcePin()->IsInputPin())
	{
		Result = -Result;
	}

    return this->IsDrawingVertical() ? FVector2D(0.0f, Result) : FVector2D(Result, 0.0f);
}

TArray<FVector2D> UDefaultNodeConnection::CalculateSubwayPoints()
{
    FVector2D SourcePosition = CachedWireData.SourcePosition;
    FVector2D TargetPosition = CachedWireData.TargetPosition;

    float DefaultOffset = GetPinOffset();

    // Calculate dynamic offset based on the Boolean flag
    float Offset = this->IsDrawingVertical()
        ? ((TargetPosition.Y - SourcePosition.Y) / 100.0f) * DefaultOffset
        : ((TargetPosition.X - SourcePosition.X) / 100.0f) * DefaultOffset;

    if (Offset <= DefaultOffset) {
        Offset = DefaultOffset;
    }

    FVector2D FirstControlPoint = this->IsDrawingVertical()
        ? FVector2D(SourcePosition.X, SourcePosition.Y + Offset)
        : FVector2D(SourcePosition.X + Offset, SourcePosition.Y);

    FVector2D SecondControlPoint = this->IsDrawingVertical()
        ? FVector2D(TargetPosition.X, TargetPosition.Y - DefaultOffset)
        : FVector2D(TargetPosition.X - DefaultOffset, TargetPosition.Y);

    return {
        SourcePosition,
        FirstControlPoint,
        SecondControlPoint,
        TargetPosition
    };
}