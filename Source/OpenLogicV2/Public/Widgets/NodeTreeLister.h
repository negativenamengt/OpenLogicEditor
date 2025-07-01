// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Classes/NodePaletteTaskEntry.h"
#include "Components/TreeView.h"
#include "NodeTreeLister.generated.h"

class UOpenLogicTask;
// Forward Declarations
class UNodePaletteBaseItem;
class UNodePaletteTaskEntry;
class UNodeTreeCategoryItem;
class UExecutionPinBase;

USTRUCT()
struct OPENLOGICV2_API FTreeListerFilterState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Search;

	UPROPERTY()
	UExecutionPinBase* PinContext = nullptr;

	bool HasSearch() const { return !Search.IsEmpty(); }
	bool HasPin() const { return PinContext != nullptr; }
	bool IsActive() const { return HasSearch() || HasPin(); }
};

UCLASS()
class OPENLOGICV2_API UNodeTreeLister : public UTreeView
{
	GENERATED_BODY()

public:
	UNodeTreeLister(const FObjectInitializer& ObjectInitializer);

	#if WITH_EDITOR
		virtual const FText GetPaletteCategory() override;
	#endif
	
	UFUNCTION(BlueprintCallable, Category = TreeLister)
	bool AddTaskEntry(UNodePaletteTaskEntry* Entry);

	// Removes a task from the tree lister.
	UFUNCTION(BlueprintCallable, Category = TreeLister)
	bool RemoveTaskEntry(UNodePaletteTaskEntry* TaskItem);

	// Processes a full refresh of the tree lister.
	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void FilterByLabel(const FString& NewSearchString);

	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void FilterByPin(UExecutionPinBase* Pin);

	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void ClearSearch();

	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void ClearPin();

	// Clears all filters applied to the tree lister.
	UFUNCTION(BlueprintCallable, Category = TreeLister)
	void ClearFilters();

	UFUNCTION(BlueprintCallable, Category = TreeLister)
	TArray<UObject*> GetChildren(UNodePaletteBaseItem* Item);

protected:
	void ApplyFilters();
	void GatherMatches();
	bool Matches(const UNodePaletteTaskEntry* Entry) const;
	bool MatchesLabel(const UNodePaletteTaskEntry* Entry) const;
	bool MatchesPin(const UNodePaletteTaskEntry* Entry) const;

	UFUNCTION(BlueprintPure, Category = TreeLister)
	FString Normalize(const FString& String) const;

	void SetExpansionRecursively(UNodeTreeCategoryItem* Category, bool bExpand);

	UPROPERTY()
	FTreeListerFilterState FilterState;

	UPROPERTY()
	TSet<UNodePaletteTaskEntry*> VisibleTasks;

	UPROPERTY()
	TSet<UNodeTreeCategoryItem*> VisibleCategories;

	UPROPERTY()
	TArray<UNodePaletteTaskEntry*> RootTasks;

	UPROPERTY()
	TArray<UNodeTreeCategoryItem*> RootCategories;

	UPROPERTY()
	TMap<TSubclassOf<UOpenLogicTask>, UNodePaletteTaskEntry*> TaskMap;
	
	UPROPERTY()
	TMap<FString, UNodeTreeCategoryItem*> CategoryMap;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "OpenLogic", meta=(ExposeOnSpawn = true))
	bool bShowCategories = true;

private:
	bool AddTaskEntryToCategory(UNodePaletteTaskEntry* Entry, const FString& CategoryPath);
	UNodeTreeCategoryItem* GetOrCreateCategory(const FString& CategoryPath);
	UNodeTreeCategoryItem* CreateNewCategory(const FString& CategoryPath);
	TArray<FString> ParseCategoryPath(const FString& CategoryPath) const;
	UNodeTreeCategoryItem* GetPrimaryCategoryForTask(UNodePaletteTaskEntry* Entry) const;
	TArray<UNodeTreeCategoryItem*> GetCategoryHierarchyForTask(UNodePaletteTaskEntry* Entry);
	
	// Whether the tree lister is currently refreshing.
	UPROPERTY()
	bool IsRefreshing = false;
};
