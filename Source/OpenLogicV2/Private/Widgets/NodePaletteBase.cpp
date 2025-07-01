// Copyright 2024 - NegativeNameSeller

#include "Widgets/NodePaletteBase.h"
#include "Tasks/OpenLogicTask.h"
#include "Classes/NodePaletteBaseItem.h"
#include "Classes/NodePaletteTaskEntry.h"
#include "Classes/NodeTreeCategoryItem.h"
#include "Engine/AssetManager.h"
#include "Tasks/OpenLogicTaskCollection.h"
#include "Widgets/NodeTreeLister.h"
#include "Widgets/GraphEditorBase.h"

void UNodePaletteBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Prevent multiple initializations
	if (bIsInitialized)
	{
		return;
	}
	bIsInitialized = true;

	if (NodeTreeLister)
	{
		NodeTreeLister->SetOnGetItemChildren(this, &UNodePaletteBase::OnGetItemChildren);
	}

	OnVisibilityChanged.AddDynamic(this, &UNodePaletteBase::OnPaletteVisibilityChanged);

	FetchTasks();
}

void UNodePaletteBase::OnPaletteVisibilityChanged_Implementation(ESlateVisibility InVisibility)
{
	if (!NodeTreeLister || !GraphEditor)
	{
		return;
	}
	
	if (IsVisible())
	{
		SetInitialNodePosition(GraphEditor->GetGraphMousePosition());
		NodeTreeLister->FilterByPin(ExecutionPinContext);
	} else
	{
		NodeTreeLister->ClearFilters();
		GraphEditor->DestroyConnectionPreview();
		ExecutionPinContext = nullptr;
	}
}

void UNodePaletteBase::AddTaskToPalette(TSubclassOf<UOpenLogicTask> TaskToAdd, FString CustomName, FString Metadata, bool bIgnoreChecks)
{
	if (!TaskToAdd)
	{
		return;
	}

	UOpenLogicTask* DefaultTask = Cast<UOpenLogicTask>(TaskToAdd->GetDefaultObject());
	if (!bIgnoreChecks && !CanShowTask(DefaultTask))
	{
		return;
	}

	FString TaskLabel = CustomName.IsEmpty() ? DefaultTask->TaskData.Name : CustomName;

	/* Creating the node palette item object */
	UNodePaletteTaskEntry* TaskEntry = NewObject<UNodePaletteTaskEntry>(this, UNodePaletteTaskEntry::StaticClass());
	TaskEntry->TaskClass = TaskToAdd;
	TaskEntry->Metadata = Metadata;
	TaskEntry->Label = TaskLabel;

	bool bSuccess;
	OnTaskAdd(TaskEntry, bSuccess);

	if (bSuccess)
	{
		TaskEntries.Add(TaskEntry);
	}
}

void UNodePaletteBase::RemoveTaskFromPalette(FString TaskName)
{
	TArray<UNodePaletteTaskEntry*> TempEntries = TaskEntries;

	for (UNodePaletteTaskEntry* TaskEntry : TempEntries)
	{
		if (!TaskEntry || TaskEntry->Label != TaskName)
		{
			continue;
		}

		bool bSuccess;
		this->OnTaskRemove(TaskEntry, bSuccess);

		if (bSuccess)
		{
			TaskEntries.Remove(TaskEntry);
		}
	}
}

void UNodePaletteBase::FetchTasks()
{
	TArray<TSoftClassPtr<UOpenLogicTask>> TaskClasses;

	if (bIsEditor)
	{
		TaskClasses = GraphEditor->GetTasksToLoad();
	}
	else {
		for (UOpenLogicTaskCollection* Collection : GraphEditor->TaskCollections)
		{
			for (TSoftClassPtr<UOpenLogicTask> TaskSoftClass : Collection->CollectionTasks)
			{
				TaskClasses.Add(TaskSoftClass);
			}
		}
	}

	TotalTasksToLoad = TaskClasses.Num();
	LoadedTasksCount = 0;

	for (TSoftClassPtr<UOpenLogicTask> TaskClass : TaskClasses)
	{
		// Start asynchronous loading with a callback to OnTaskLoaded
		FStreamableManager& AssetLoader = UAssetManager::GetStreamableManager();
		AssetLoader.RequestAsyncLoad(TaskClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UNodePaletteBase::OnTaskLoaded, TaskClass));
	}
}

UNodeBase* UNodePaletteBase::TryCreateNode(UNodePaletteTaskEntry* TaskEntry)
{
	if (!GraphEditor || !TaskEntry)
	{
		return nullptr;
	}

	UNodeBase* Node = GraphEditor->CreateNode(TaskEntry->TaskClass, GraphEditor->GetGraphMousePosition());
	if (!Node)
	{
		return nullptr;
	}

	OnTaskNodePlacedInGraph.Broadcast(Node, TaskEntry);

	if (!ExecutionPinContext)
	{
		GraphEditor->HideNodePalette(this);
		return Node;
	}

	TArray<FOpenLogicConnectablePin> Matches = Node->TaskObject->GetConnectablePins(ExecutionPinContext->GetPinInfo(), ExecutionPinContext->GetPinDirection());
	if (Matches.Num() > 0)
	{
		const int32 Index = Matches[0].PinIndex + 1;

		UExecutionPinBase* InPin  = ExecutionPinContext->IsInputPin()
			? ExecutionPinContext
			: (Node->InputPins.Contains(Index) ? Node->InputPins[Index] : nullptr);

		UExecutionPinBase* OutPin = ExecutionPinContext->IsInputPin()
			? (Node->OutputPins.Contains(Index) ? Node->OutputPins[Index] : nullptr)
			: ExecutionPinContext;

		if (InPin && OutPin)
		{
			GraphEditor->DestroyConnectionPreview();
			GraphEditor->CreateConnection(OutPin, InPin, false, true);
		}
	}

	GraphEditor->HideNodePalette(this);
	return Node;
}

void UNodePaletteBase::OnTaskAdd_Implementation(UNodePaletteTaskEntry* EntryItem, bool& Success)
{
	if (NodeTreeLister)
	{
		Success = NodeTreeLister->AddTaskEntry(EntryItem);
	}
}

void UNodePaletteBase::OnTaskRemove_Implementation(UNodePaletteTaskEntry* EntryItem, bool& Success)
{
	if (NodeTreeLister)
	{
		Success = NodeTreeLister->RemoveTaskEntry(EntryItem);
	}
}

void UNodePaletteBase::UpdateTreeFilterByCurrentPin()
{
}

void UNodePaletteBase::OnTaskLoaded(TSoftClassPtr<UOpenLogicTask> TaskClass)
{
	AddTaskToPalette(TaskClass.Get());

	LoadedTasksCount++;

	if (LoadedTasksCount >= TotalTasksToLoad)
	{
		// All tasks have been loaded
		OnAllTasksLoaded();
	}
}

void UNodePaletteBase::OnAllTasksLoaded() const
{
	if (NodeTreeLister)
	{
		NodeTreeLister->Refresh();
	}
}

bool UNodePaletteBase::CanShowTask(UOpenLogicTask* Task)
{
	if (!Task->ShowInNodePalette || !GraphEditor)
	{
		return false;
	}

	UOpenLogicGraph* Graph = GraphEditor->GetGraph();

	if (!Graph)
	{
		return false;
	}

	return Graph->ImportedLibraries.HasAny(Task->TaskData.Library);
}

void UNodePaletteBase::OnGetItemChildren(UObject* Item, TArray<UObject*>& OutChildren)
{
	UNodeTreeCategoryItem* CategoryItem = Cast<UNodeTreeCategoryItem>(Item);
	if (!CategoryItem || !NodeTreeLister)
	{
		return;
	}
	
	OutChildren = NodeTreeLister->GetChildren(CategoryItem);
}
