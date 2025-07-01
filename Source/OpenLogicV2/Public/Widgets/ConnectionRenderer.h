// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ConnectionRenderer.generated.h"

class UGraphEditorBase;

UCLASS()
class OPENLOGICV2_API UConnectionRenderer : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "OpenLogic")
		bool bDisplayDebugCells = false;

public:
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
};