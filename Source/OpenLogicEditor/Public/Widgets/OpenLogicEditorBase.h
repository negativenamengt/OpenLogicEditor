// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "InspectorBase.h"
#include "Widgets/GraphEditorBase.h"
#include "Widgets/NodeBase.h"
#include "Classes/OpenLogicGraph.h"
#include "Classes/GraphCustomization.h"
#include "UObject/UObjectGlobals.h"
#include "OpenLogicEditorBase.generated.h"

UCLASS(Blueprintable, Abstract)
class OPENLOGICEDITOR_API UOpenLogicEditorBase : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void SetupEditor(UGraphEditorBase* GraphEditor);

private:
	UFUNCTION()
		void SaveGraphAsset();

	// Function called when a node is added to the graph editor.
	UFUNCTION()
		void OnNodeAdded_Internal(UNodeBase* Node);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenLogic")
		UOpenLogicGraph* GraphObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenLogic")
		UGraphCustomization* GraphStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OpenLogic")
		UInspectorBase* NodeInspectorWidget;

private:
	UPROPERTY()
		UGraphEditorBase* GraphEditorReference;

	UPROPERTY()
		bool bIsSetup = false;

	UPROPERTY()
		bool bIsRecompiling = false;

protected:
	virtual void NativeDestruct() override;
};