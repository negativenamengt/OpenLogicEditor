// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/GraphEditorBase.h"
#include "OpenLogicGraphEditorManager.generated.h"

UCLASS()
class OPENLOGICV2_API UOpenLogicGraphEditorManager : public UObject
{
	GENERATED_BODY()

public:
	static UOpenLogicGraphEditorManager* Get();

public:
	// Adds a graph editor to the manager
	void AddGraphEditor(UGraphEditorBase* GraphEditor);

	// Removes a graph editor from the manager
	void RemoveGraphEditor(UGraphEditorBase* GraphEditor);

	// Returns all graph editors
	const TSet<UGraphEditorBase*>& GetGraphEditors() const;

private:
	// List of all graph editors
	TSet<UGraphEditorBase*> GraphEditors;

	static UOpenLogicGraphEditorManager* Instance;

};
