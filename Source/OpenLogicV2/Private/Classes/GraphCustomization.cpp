// Copyright 2024 - NegativeNameSeller

#include "Classes/GraphCustomization.h"

UGraphCustomization::UGraphCustomization(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SelectionBoxBrush = FSlateBrush();
	SelectionBoxBrush.DrawAs = ESlateBrushDrawType::Border;
	SelectionBoxBrush.TintColor = FLinearColor::White;
	SelectionBoxBrush.Margin = FMargin(1);
}