// Copyright 2024 - NegativeNameSeller

#include "Widgets/GraphGridRenderer.h"
#include "Widgets/GraphEditorBase.h"
#include "Classes/GraphCustomization.h"

void UGraphGridRenderer::DrawRulerLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const
{
	UGraphCustomization* Customization = GetCustomization();

	const float RulerGridSpacing = Customization->RulerGridSpacing;
	const FLinearColor RulerGridColor = Customization->RulerGridColor;
	const float RulerGridThickness = Customization->RulerGridThickness;

	float StartX = FMath::Fmod(Offset.X, RulerGridSpacing);
	float StartY = FMath::Fmod(Offset.Y, RulerGridSpacing);

	for (float X = StartX; X < Size.X; X += RulerGridSpacing)
	{
		if (X <= 0.0f || X >= Size.X)
			continue;

		DrawLines(Context, FVector2D(X, 0), FVector2D(X, Size.Y), RulerGridColor, RulerGridThickness);
	}

	for (float Y = StartY; Y < Size.Y; Y += RulerGridSpacing)
	{
		if (Y <= 0.0f || Y >= Size.Y)
			continue;

		DrawLines(Context, FVector2D(0, Y), FVector2D(Size.X, Y), RulerGridColor, RulerGridThickness);
	}
}

void UGraphGridRenderer::DrawMinorGridLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const
{
	UGraphCustomization* Customization = GetCustomization();

	float MinorGridSpacing = Customization->RulerGridSpacing / 8.0f;
	const FLinearColor MinorGridColor = Customization->MinorGridColor;
	const float MinorGridThickness = Customization->MinorGridThickness;

	float StartX = FMath::Fmod(Offset.X, MinorGridSpacing);
	float StartY = FMath::Fmod(Offset.Y, MinorGridSpacing);

	for (float X = StartX; X < Size.X; X += MinorGridSpacing)
	{
		if (X <= 0.0f || X >= Size.X)
			continue;

		DrawLines(Context, FVector2D(X, 0), FVector2D(X, Size.Y), MinorGridColor, MinorGridThickness);
	}

	for (float Y = StartY; Y < Size.Y; Y += MinorGridSpacing)
	{
		if (Y <= 0.0f || Y >= Size.Y)
			continue;

		DrawLines(Context, FVector2D(0, Y), FVector2D(Size.X, Y), MinorGridColor, MinorGridThickness);
	}
}

void UGraphGridRenderer::DrawOriginLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const
{
	UGraphCustomization* Customization = GetCustomization();

	const FLinearColor OriginColor = Customization->OriginColor;
	const float OriginThickness = Customization->OriginThickness;

	DrawLines(Context, FVector2D(Offset.X, 0), FVector2D(Offset.X, Size.Y), OriginColor, OriginThickness);
	DrawLines(Context, FVector2D(0, Offset.Y), FVector2D(Size.X, Offset.Y), OriginColor, OriginThickness);
}

void UGraphGridRenderer::DrawLines(const FPaintContext& Context, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, const float Thickness) const
{
    FSlateDrawElement::MakeLines(
		Context.OutDrawElements,
		Context.LayerId,
		Context.AllottedGeometry.ToPaintGeometry(),
		{ Start, End },
		ESlateDrawEffect::None,
		Color,
		true,
		Thickness
	);
}

UGraphCustomization* UGraphGridRenderer::GetCustomization() const
{
	if (!OwningGraphEditor)
		return nullptr;

	return OwningGraphEditor->GraphStyleAsset;
}

int32 UGraphGridRenderer::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	UGraphCustomization* Customization = GetCustomization();
    if (!OwningGraphEditor || !Customization)
        return LayerId;

    const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
    const FVector2D Offset = OwningGraphEditor->GetGraphPosition();

	FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (Customization->ShowMinorGrid)
		this->DrawMinorGridLines(Context, Offset, LocalSize);

	if (Customization->ShowRulerGrid)
		this->DrawRulerLines(Context, Offset, LocalSize);

	if (Customization->ShowOrigin)
		this->DrawOriginLines(Context, Offset, LocalSize);

    return LayerId;
}
