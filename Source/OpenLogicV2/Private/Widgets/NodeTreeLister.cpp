// Copyright 2024 - NegativeNameSeller

#include "Widgets/NodeTreeLister.h"
#include "Widgets/ExecutionPinBase.h"
#include "Widgets/NodeBase.h"
#include "Classes/NodePaletteBaseItem.h"
#include "Classes/NodePaletteTaskEntry.h"
#include "Classes/NodeTreeCategoryItem.h"
#include "Tasks/OpenLogicTask.h"

UNodeTreeLister::UNodeTreeLister(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SelectionMode = ESelectionMode::None;
}

#if WITH_EDITOR

const FText UNodeTreeLister::GetPaletteCategory()
{
	return FText::FromString("OpenLogic");
}

#endif


bool UNodeTreeLister::AddTaskEntry(UNodePaletteTaskEntry* Entry)
{
	if (!Entry || TaskMap.Contains(Entry->TaskClass))
	{
		return false;
	}

	UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(Entry->TaskClass->GetDefaultObject());
	if (!TaskDefinition)
	{
		return false;
	}

	const FString& CategoryPath = TaskDefinition->TaskData.Category;
	if (bShowCategories && !CategoryPath.IsEmpty())
	{
		AddTaskEntryToCategory(Entry, CategoryPath);
	} else
	{
		RootTasks.Add(Entry);
	}

	TaskMap.Add(Entry->TaskClass, Entry);

	return true;
}

bool UNodeTreeLister::RemoveTaskEntry(UNodePaletteTaskEntry* TaskItem)
{
	if (!TaskItem)
	{
		return false;
	}

	UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(TaskItem->TaskClass->GetDefaultObject());
	if (!TaskDefinition)
	{
		return false;
	}

	const FString& CategoryPath = TaskDefinition->TaskData.Category;
	if (!bShowCategories || CategoryPath.IsEmpty())
	{
		RootTasks.Remove(TaskItem);
	}
	else
	{
		UNodeTreeCategoryItem* CategoryItem = CategoryMap.FindRef(CategoryPath);
		if (CategoryItem)
		{
			CategoryItem->Childrens.Remove(TaskItem);
		}
	}

	TaskMap.Remove(TaskItem->TaskClass);

	return true;
}

void UNodeTreeLister::Refresh()
{
	TArray<UObject*> AllItems;

	// Sort and append the root categories
	RootCategories.Sort([](const UNodeTreeCategoryItem& A, const UNodeTreeCategoryItem& B)
	{
		return A.Label < B.Label;
	});

	for (UNodePaletteBaseItem* Category : RootCategories)
	{
		if (Category->IsMatchingFilter)
		{
			AllItems.Add(Category);
		}
	}

	// Sort and append the root tasks
	RootTasks.Sort([](const UNodePaletteTaskEntry& A, const UNodePaletteTaskEntry& B)
	{
		return A.Label < B.Label;
	});

	for (UNodePaletteBaseItem* Task : RootTasks)
	{
		if (Task->IsMatchingFilter)
		{
			AllItems.Add(Task);
		}
	}

	SetListItems(AllItems);
}

void UNodeTreeLister::FilterByLabel(const FString& NewSearchString)
{
	if (NewSearchString.IsEmpty())
	{
		ClearSearch();
		CollapseAll();
		ApplyFilters();
	}
	else
	{
		FilterState.Search = Normalize(NewSearchString);
		ApplyFilters();
	}
}

void UNodeTreeLister::FilterByPin(UExecutionPinBase* Pin)
{
	FilterState.PinContext = Pin;
	ApplyFilters();
}

void UNodeTreeLister::ClearSearch()
{
	FilterState.Search.Empty();
}

void UNodeTreeLister::ClearPin()
{
	FilterState.PinContext = nullptr;
}

void UNodeTreeLister::ClearFilters()
{
	ClearSearch();
	ClearPin();
	ApplyFilters();
}

TArray<UObject*> UNodeTreeLister::GetChildren(UNodePaletteBaseItem* Item)
{
	UNodeTreeCategoryItem* CategoryItem = Cast<UNodeTreeCategoryItem>(Item);
	if (!CategoryItem)
	{
		return TArray<UObject*>();
	}

	TArray<UObject*> Children;
	for (int32 Index = 0; Index < CategoryItem->Childrens.Num(); ++Index)
	{
		UObject* Child = CategoryItem->Childrens[Index];
		UNodePaletteBaseItem* PaletteItem = Cast<UNodePaletteBaseItem>(Child);
		if (PaletteItem != nullptr)
		{
			if (!FilterState.IsActive() || PaletteItem->IsMatchingFilter)
			{
				Children.Add(Child);
			}
		}
	}

	return Children;
}

void UNodeTreeLister::ApplyFilters()
{
	VisibleTasks.Empty();
	VisibleCategories.Empty();

	// Reset task flags
	for (TPair<TSubclassOf<UOpenLogicTask>, UNodePaletteTaskEntry*>& Pair : TaskMap)
	{
		if (Pair.Value)
		{
			Pair.Value->IsMatchingFilter = false;
		}
	}

	// Reset category flags
	for (TPair<FString, UNodeTreeCategoryItem*>& CatPair : CategoryMap)
	{
		if (CatPair.Value)
		{
			CatPair.Value->IsMatchingFilter = false;
		}
	}

	if (!FilterState.IsActive())
	{
		for (TPair<TSubclassOf<UOpenLogicTask>, UNodePaletteTaskEntry*>& Pair : TaskMap)
		{
			if (Pair.Value)
			{
				Pair.Value->IsMatchingFilter = true;
			}
		}

		for (TPair<FString, UNodeTreeCategoryItem*>& CatPair : CategoryMap)
		{
			if (CatPair.Value)
			{
				CatPair.Value->IsMatchingFilter = true;
			}
		}

		Refresh();

		// Collapse all categories
		if (MyTreeView.IsValid())
		{
			MyTreeView->ClearExpandedItems();
		}
		RegenerateAllEntries();
		return;
	}

	GatherMatches();
	Refresh();

	// Expand matching categories
	for (auto& Category : VisibleCategories)
	{
		SetItemExpansion(Category, true);
	}
}

void UNodeTreeLister::GatherMatches()
{
	// Evaluate the tasks
	for (auto& Pair : TaskMap)
	{
		UNodePaletteTaskEntry* TaskEntry = Pair.Value;
		if (!TaskEntry)
		{
			continue;
		}

		bool bMatches = Matches(TaskEntry);
		TaskEntry->IsMatchingFilter = bMatches;

		if (bMatches)
		{
			VisibleTasks.Add(TaskEntry);
		}
	}

	// Identity the categories for the visible tasks
	for (UNodePaletteTaskEntry* TaskEntry : VisibleTasks)
	{
		UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(TaskEntry->TaskClass->GetDefaultObject());
		if (!TaskDefinition)
		{
			continue;
		}

		for (UNodeTreeCategoryItem* Category : GetCategoryHierarchyForTask(TaskEntry))
		{
			if (!Category)
			{
				continue;
			}

			Category->IsMatchingFilter = true;
			VisibleCategories.Add(Category);
		}
	}
}

bool UNodeTreeLister::Matches(const UNodePaletteTaskEntry* Entry) const
{
	bool bMatchesLabel = !FilterState.HasSearch() || MatchesLabel(Entry);
	bool bMatchesPin = !FilterState.HasPin() || MatchesPin(Entry);
	return bMatchesLabel && bMatchesPin;
}

bool UNodeTreeLister::MatchesLabel(const UNodePaletteTaskEntry* Entry) const
{
	return Normalize(Entry->Label).Contains(FilterState.Search);
}

bool UNodeTreeLister::MatchesPin(const UNodePaletteTaskEntry* Entry) const
{
	if (!FilterState.HasPin())
	{
		return true;
	}

	UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(Entry->TaskClass->GetDefaultObject());
	if (!TaskDefinition)
	{
		return false;
	}

	UExecutionPinBase* PinContext = FilterState.PinContext;
	if (!PinContext)
	{
		return false;
	}

	TArray<FOpenLogicConnectablePin> ConnectablePins = TaskDefinition->GetConnectablePins(PinContext->GetPinInfo(), PinContext->GetPinDirection());
	return !ConnectablePins.IsEmpty();
}

FString UNodeTreeLister::Normalize(const FString& String) const
{
	return String.Replace(TEXT(" "), TEXT("")).ToLower();
}

void UNodeTreeLister::SetExpansionRecursively(UNodeTreeCategoryItem* Category, bool bExpand)
{
	if (Category == nullptr)
	{
		return;
	}

	TQueue<UNodeTreeCategoryItem*> Queue;
	Queue.Enqueue(Category);
	UNodeTreeCategoryItem* Current = nullptr;

	while (Queue.Dequeue(Current))
	{
		SetItemExpansion(Current, bExpand);
		for (int32 i = 0; i < Current->Childrens.Num(); ++i)
		{
			UNodePaletteBaseItem* ChildItem = Current->Childrens[i];
			UNodeTreeCategoryItem* ChildCategory = Cast<UNodeTreeCategoryItem>(ChildItem);
			if (ChildCategory != nullptr)
			{
				Queue.Enqueue(ChildCategory);
			}
		}
	}
}

bool UNodeTreeLister::AddTaskEntryToCategory(UNodePaletteTaskEntry* Entry, const FString& CategoryPath)
{
	if (!Entry || CategoryPath.IsEmpty())
	{
		return false;
	}

	UNodeTreeCategoryItem* CategoryItem = GetOrCreateCategory(CategoryPath);
	if (!CategoryItem)
	{
		return false;
	}

	CategoryItem->Childrens.Add(Entry);
	return true;
}

UNodeTreeCategoryItem* UNodeTreeLister::GetOrCreateCategory(const FString& CategoryPath)
{
	if (UNodeTreeCategoryItem* CategoryItem = CategoryMap.FindRef(CategoryPath))
	{
		return CategoryItem;
	}

	return CreateNewCategory(CategoryPath);
}

UNodeTreeCategoryItem* UNodeTreeLister::CreateNewCategory(const FString& CategoryPath)
{
	TArray<FString> Levels = ParseCategoryPath(CategoryPath);
	UNodeTreeCategoryItem* Parent = nullptr;
	FString AccumulatedPath;

	for (int32 Index = 0; Index < Levels.Num(); ++Index)
	{
		const FString& Level = Levels[Index];
		if (AccumulatedPath.IsEmpty())
		{
			AccumulatedPath = Level;
		}
		else
		{
			AccumulatedPath += TEXT("|") + Level;
		}

		UNodeTreeCategoryItem*& Ref = CategoryMap.FindOrAdd(AccumulatedPath);
		if (Ref == nullptr)
		{
			Ref = NewObject<UNodeTreeCategoryItem>(this);
			Ref->Label = AccumulatedPath;
			if (Parent != nullptr)
			{
				Parent->Childrens.Add(Ref);
			}
			else
			{
				RootCategories.Add(Ref);
			}
		}
		Parent = Ref;
	}

	return Parent;
}

TArray<FString> UNodeTreeLister::ParseCategoryPath(const FString& CategoryPath) const
{
	TArray<FString> Out;
	CategoryPath.ParseIntoArray(Out, TEXT("|"), true);
	return Out;
}

UNodeTreeCategoryItem* UNodeTreeLister::GetPrimaryCategoryForTask(UNodePaletteTaskEntry* Entry) const
{
	if (!Entry)
	{
		return nullptr;
	}

	UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(Entry->TaskClass->GetDefaultObject());
	if (!TaskDefinition)
	{
		return nullptr;
	}

	UNodeTreeCategoryItem* CategoryItem = CategoryMap.FindRef(TaskDefinition->TaskData.Category);
	if (CategoryItem)
	{
		return CategoryItem;
	}

	return nullptr;
}

TArray<UNodeTreeCategoryItem*> UNodeTreeLister::GetCategoryHierarchyForTask(UNodePaletteTaskEntry* Entry)
{
	TArray<UNodeTreeCategoryItem*> Hierarchy;
	if (!Entry)
	{
		return Hierarchy;
	}

	UOpenLogicTask* TaskDefinition = Cast<UOpenLogicTask>(Entry->TaskClass->GetDefaultObject());
	if (!TaskDefinition)
	{
		return Hierarchy;
	}

	FString AccumulatedPath;
	TArray<FString> Levels = ParseCategoryPath(TaskDefinition->TaskData.Category);
	for (int32 Index = 0; Index < Levels.Num(); ++Index)
	{
		const FString& Level = Levels[Index];
		if (AccumulatedPath.IsEmpty())
		{
			AccumulatedPath = Level;
		}
		else
		{
			AccumulatedPath += TEXT("|") + Level;
		}

		UNodeTreeCategoryItem* CategoryItem = CategoryMap.FindRef(AccumulatedPath);
		if (CategoryItem)
		{
			Hierarchy.Add(CategoryItem);
		}
	}
	return Hierarchy;
}
