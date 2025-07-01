// Copyright 2024 - NegativeNameSeller

#include "Classes/OpenLogicGraphEditorManager.h"

UOpenLogicGraphEditorManager* UOpenLogicGraphEditorManager::Instance = nullptr;

UOpenLogicGraphEditorManager* UOpenLogicGraphEditorManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UOpenLogicGraphEditorManager>();
		Instance->AddToRoot();
	}

	return Instance;
}

void UOpenLogicGraphEditorManager::AddGraphEditor(UGraphEditorBase* GraphEditor)
{
	if (GraphEditor)
	{
		GraphEditors.Add(GraphEditor);
	}
}

void UOpenLogicGraphEditorManager::RemoveGraphEditor(UGraphEditorBase* GraphEditor)
{
	if (GraphEditor)
	{
		GraphEditors.Remove(GraphEditor);
	}
}

const TSet<UGraphEditorBase*>& UOpenLogicGraphEditorManager::GetGraphEditors() const
{
	return GraphEditors;
}
