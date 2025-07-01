// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NodePaletteBaseItem.generated.h"

UCLASS(Blueprintable)
class OPENLOGICV2_API UNodePaletteBaseItem : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "OpenLogic", meta=(ExposeOnSpawn))
		FString Label;

	UPROPERTY()
		bool IsMatchingFilter = true;
};
