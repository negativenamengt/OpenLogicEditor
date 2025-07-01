// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/OpenLogicTypes.h"
#include "OpenLogicGraph.generated.h"

class UGraphCustomization;

UCLASS(Blueprintable)
class OPENLOGICV2_API UOpenLogicGraph : public UObject
{
	GENERATED_BODY()
public:
	static TSoftObjectPtr<UGraphCustomization> GetDefaultGraphCustomization();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = OpenLogic)
		FOpenLogicGraphData GraphData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic, meta = (ExposeOnSpawn = true))
		FGameplayTagContainer ImportedLibraries = FGameplayTagContainer(TAG_OpenLogicUserLibrary);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic, meta = (ExposeOnSpawn = true))
		FName OptionalGraphName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = OpenLogic, NoClear, meta = (ExposeOnSpawn = true))
		TSoftObjectPtr<UGraphCustomization> GraphCustomization = GetDefaultGraphCustomization();
public:
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		bool FromString(UPARAM(DisplayName = "Serialized String") FString Json);
	
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		FString ToString();

public:
	// Migrates the graph data to the latest schema version.
	// Returns true if the graph data needed to be migrated.
	UFUNCTION(BlueprintCallable, Category = OpenLogic)
		bool MigrateToLatestSchemaVersion();
};