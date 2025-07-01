// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tasks/OpenLogicTask.h"
#include "Tasks/OpenLogicTaskCollection.h"
#include "OpenLogicEditorUtility.generated.h"

UCLASS(Blueprintable)
class OPENLOGICEDITOR_API UOpenLogicEditorUtility : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		static void MarkTaskCollectionDirty(UOpenLogicTaskCollection* TaskCollection);

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		static TArray<FString> GetTaskAssetPaths();

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		static TArray<TSoftClassPtr<UOpenLogicTask>> GetAllTaskSoftClasses();
};
