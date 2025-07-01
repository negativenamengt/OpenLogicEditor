// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Classes/NodePaletteBaseItem.h"
#include "NodeTreeCategoryItem.generated.h"

UCLASS()
class OPENLOGICV2_API UNodeTreeCategoryItem : public UNodePaletteBaseItem
{
	GENERATED_BODY()

public:
	UPROPERTY()
		TArray<UNodePaletteBaseItem*> Childrens;

	UPROPERTY()
		UNodeTreeCategoryItem* ParentCategory;

	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic")
		bool IsExpanded = false;
};
