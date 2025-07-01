// Copyright 2024 - NegativeNameSeller

#include "Widgets/InspectorBase.h"
#include "Widgets/NodeBase.h"

void UInspectorBase::NativeConstruct()
{
	if (!PayloadEditor)
	{
		return;
	}

}

void UInspectorBase::OnNodeSelectionChanged(UNodeBase* Node, bool IsSelected)
{
	// Ensure the details view is valid.
	if (!PayloadEditor || !Node ||!Node->TaskObject)
	{
		return;
	}

	if (IsSelected)
	{
		// Set the object to the details view.
		TArray<UNodeBase*> Nodes = PayloadEditor->GetNodes();
		Nodes.Add(Node);

		PayloadEditor->SetNodes(Nodes);

		return;
	}

	PayloadEditor->SetNodes({});
}

void UInspectorBase::AlertPropertyChange(FName PropertyName)
{

}