// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphGridRenderer.generated.h"

class UGraphEditorBase;
class UGraphCustomization;

UCLASS()
class OPENLOGICV2_API UGraphGridRenderer : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

public:
	// Draws the ruler grid lines in the given paint context.
	UFUNCTION()
		void DrawRulerLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const;

	// Draws the minor grid lines in the given paint context.
	UFUNCTION()
		void DrawMinorGridLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const;

	// Draws the origin lines in the given paint context.
	UFUNCTION()
		void DrawOriginLines(const FPaintContext& Context, const FVector2D& Offset, const FVector2D& Size) const;

protected:
	// Draws lines in the given paint context.
	UFUNCTION()
		void DrawLines(const FPaintContext& Context, const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, const float Thickness) const;

	UFUNCTION()
		UGraphCustomization* GetCustomization() const;

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
	
};
	