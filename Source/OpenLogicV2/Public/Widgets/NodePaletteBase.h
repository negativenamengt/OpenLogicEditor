// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NodePaletteBase.generated.h"

class UNodeTreeLister;
// Forward declarations
class UGraphEditorBase;
class UOpenLogicTask;
class UExecutionPinBase;

class UNodePaletteBaseItem;
class UNodePaletteTaskEntry;

// Dispatchers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskNodePlacedInGraph, UNodeBase*, Node, UNodePaletteTaskEntry*, TaskEntry);

UCLASS(Blueprintable, Abstract)
class OPENLOGICV2_API UNodePaletteBase : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic|NodePalette")
	void OnPaletteVisibilityChanged(ESlateVisibility InVisibility);
	
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
	void SetInitialNodePosition(FVector2D NewInitialPosition) { InitialNodePosition = NewInitialPosition; }

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
	FVector2D GetInitialNodePosition() { return InitialNodePosition; }

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
	void AddTaskToPalette(TSubclassOf<UOpenLogicTask> TaskToAdd, FString CustomName = "", FString Metadata = "", bool bIgnoreChecks = false);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
	void RemoveTaskFromPalette(FString TaskName);

	// Fetches all the available tasks and adds them to the node palette.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
	void FetchTasks();

	// Delegate triggered when a task is added to the graph.
	UPROPERTY(BlueprintAssignable, Category = "OpenLogic|NodePalette")
	FOnTaskNodePlacedInGraph OnTaskNodePlacedInGraph;

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
	UNodeBase* TryCreateNode(UNodePaletteTaskEntry* TaskEntry);

	// Called when a task is being added to the Node Palette.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic|NodePalette")
	void OnTaskAdd(UNodePaletteTaskEntry* EntryItem, bool & Success);

	// Called when a task is being removed from the Node Palette.
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic|NodePalette")
	void OnTaskRemove(UNodePaletteTaskEntry* EntryItem, bool& Success);

	UFUNCTION()
	void SetGraphEditor(UGraphEditorBase* NewGraphEditor) { GraphEditor = NewGraphEditor; }

	UFUNCTION()
	void GetGraphEditor(UGraphEditorBase*& OutGraphEditor) const { OutGraphEditor = GraphEditor; }

	UFUNCTION()
	void SetIsEditor(bool bNewIsEditor) { bIsEditor = bNewIsEditor; }

	UFUNCTION()
	bool GetIsEditor() const { return bIsEditor; }

	UFUNCTION(BlueprintCallable, Category = "OpenLogic|NodePalette")
	void UpdateTreeFilterByCurrentPin();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "NodePalette")
	TArray<UNodePaletteTaskEntry*> TaskEntries;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic")
	bool bIsEditor;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic|NodePalette")
	UGraphEditorBase* GraphEditor;

	UPROPERTY(BlueprintReadWrite, Category = "OpenLogic")
	UExecutionPinBase* ExecutionPinContext;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UNodeTreeLister* NodeTreeLister;

private:
	UFUNCTION()
	void OnTaskLoaded(TSoftClassPtr<UOpenLogicTask> TaskClass);

	UFUNCTION()
	void OnAllTasksLoaded() const;

	UFUNCTION()
	bool CanShowTask(UOpenLogicTask* Task);

	UPROPERTY()
	FVector2D InitialNodePosition = FVector2D::Zero();

	UPROPERTY()
	bool bIsInitialized = false;

	UFUNCTION()
	void OnGetItemChildren(UObject* Item, TArray<UObject*>& OutChildren);

	UPROPERTY()
	int32 LoadedTasksCount = 0;

	UPROPERTY()
	int32 TotalTasksToLoad = 0;
};
