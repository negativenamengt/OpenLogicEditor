// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "OpenLogicPayloadEditor.h"
#include "InspectorBase.generated.h"

class UNodeBase;
class UGraphEditorBase;

UCLASS(Blueprintable, Abstract)
class OPENLOGICEDITOR_API UInspectorBase : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

public:
	// Triggered when the selection of a node changes.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void OnNodeSelectionChanged(UNodeBase* Node, bool IsSelected);

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic", meta = (BindWidget))
		UOpenLogicPayloadEditor* PayloadEditor;

public:
	// The graph editor linked to this inspector.
	UPROPERTY()
		UGraphEditorBase* LinkedGraphEditor;

private:
	// Triggered when a property of a node changes.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void AlertPropertyChange(FName PropertyName);
};