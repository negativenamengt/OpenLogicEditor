// Copyright 2024 - NegativeNameSeller

#include "Factory/Actions_OpenLogicGraph.h"
#include "Classes/OpenLogicGraph.h"
#include "OpenLogicAssetEditorToolkit.h"
#include "OpenLogicEditor.h"

UClass* FAssetType_OpenLogicGraph::GetSupportedClass() const
{
	return UOpenLogicGraph::StaticClass();
}

FText FAssetType_OpenLogicGraph::GetName() const
{
	return FText::FromString("Open Logic Graph");
}

FColor FAssetType_OpenLogicGraph::GetTypeColor() const
{
	return FColor::Cyan;
}

uint32 FAssetType_OpenLogicGraph::GetCategories()
{
	return FOpenLogicEditorModule::OpenLogic_AssetCategory;
}

void FAssetType_OpenLogicGraph::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	if (InObjects.IsEmpty())
	{
		return;
	}

	TSharedPtr<FOpenLogicAssetEditorToolkit> Toolkit = MakeShareable(new FOpenLogicAssetEditorToolkit());
	Toolkit->InitEditor(InObjects);
}