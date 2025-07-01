// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

class OPENLOGICEDITOR_API FAssetType_OpenLogicProperty : public FAssetTypeActions_Blueprint
{
public:
	virtual UClass* GetSupportedClass() const override;
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;

protected:
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;
};
