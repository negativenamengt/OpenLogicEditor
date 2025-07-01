// Copyright 2024 - NegativeNameSeller

#include "Widgets/OpenLogicEditorBase.h"
#include "Settings/OpenLogicEditorSettings.h"
#include "Utility/OpenLogicEditorUtility.h"
#include "Widgets/OpenLogicPayloadEditor.h"
#include "Subsystems/EditorAssetSubsystem.h"

void UOpenLogicEditorBase::SetupEditor(UGraphEditorBase* GraphEditor)
{
	// Make sure the editor is not already setup and that the graph object is valid
	if (bIsSetup || !GraphObject)
	{
		return;
	}

	// Make sure the graph editor widget reference is valid
	if (!GraphEditor)
	{
		return;
	}

	bIsSetup = true;

	// Set the graph editor reference
	GraphEditorReference = GraphEditor;

	// Give the graph editor to the inspector widget
	if (NodeInspectorWidget && NodeInspectorWidget->PayloadEditor)
	{
		NodeInspectorWidget->PayloadEditor->SetDefaultObject(GraphObject);
	}

	GraphEditorReference->SetTasksToLoad(UOpenLogicEditorUtility::GetAllTaskSoftClasses());

	// Bind the OnNodeSelectStateChanged event to the NodeInspectorWidget
	GraphEditorReference->OnNodeSelectStateChanged.RemoveAll(NodeInspectorWidget);
	GraphEditorReference->OnNodeSelectStateChanged.AddDynamic(NodeInspectorWidget, &UInspectorBase::OnNodeSelectionChanged);

	// Bind the OnNodeAdded event to the OnNodeAdded_Internal function
	GraphEditorReference->OnNodeAdded.RemoveAll(this);
	GraphEditorReference->OnNodeAdded.AddDynamic(this, &UOpenLogicEditorBase::OnNodeAdded_Internal);

	// Setup the GraphStyleAsset
	UOpenLogicEditorSettings* Settings = GetMutableDefault<UOpenLogicEditorSettings>();

	// Register the auto-save timer
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerHandle AutoSaveTimerHandle;
		World->GetTimerManager().SetTimer(AutoSaveTimerHandle, this, &UOpenLogicEditorBase::SaveGraphAsset, Settings->AutoSaveInSeconds, true);
		GraphEditorReference->RegisterTimer(AutoSaveTimerHandle);
	}

	// Initialize the Graph Editor
	GraphEditorReference->InitializeGraphEditor(GraphObject);
}

void UOpenLogicEditorBase::NativeDestruct()
{
	if (GraphEditorReference)
	{
		GraphEditorReference->ClearAllTimers();
	}

	Super::NativeDestruct();
}

void UOpenLogicEditorBase::SaveGraphAsset()
{
	if (!GraphObject)
	{
		return;
	}

	// If the NodePaletteWidget is visible, don't save the asset (otherwise the palette unfocuses)
	if (GraphEditorReference &&
		GraphEditorReference->GetNodePaletteWidget() &&
		GraphEditorReference->GetNodePaletteWidget()->IsVisible())
	{
		return;
	}

	if (!GraphObject->GetOutermost() || !GraphObject->GetOutermost()->IsDirty())
	{
		return;
	}

	UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	EditorAssetSubsystem->SaveLoadedAsset(GraphObject);
}

void UOpenLogicEditorBase::OnNodeAdded_Internal(UNodeBase* Node)
{
	if (!Node || !Node->TaskObject || !Node->TaskObject->ShowPayloadEditor)
	{
		return;
	}

	UOpenLogicPayloadEditor* PayloadEditor = NewObject<UOpenLogicPayloadEditor>(Node);
	PayloadEditor->bAllowSearch = false;
	PayloadEditor->SetNodes({ Node });

	Node->OnDisplayableWidgetAdded(PayloadEditor);
}
