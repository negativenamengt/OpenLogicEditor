// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Widgets/GraphEditorBase.h"
#include "Components/CanvasPanelSlot.h"
#include "GraphSelectionDragOperation.generated.h"

UCLASS(NotBlueprintable, NotPlaceable, NotBlueprintType)
class OPENLOGICV2_API UGraphSelectionDragOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UFUNCTION()
		void Initialize(UGraphEditorBase* InOwningGraphEditor);

public:
	UPROPERTY()
		UGraphEditorBase* OwningGraphEditor;

protected:
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;

protected:
	UFUNCTION()
		void ClearSelectionBox();

	UFUNCTION()
		UCanvasPanelSlot* GetSelectionBoxSlot();
};
