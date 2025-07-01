// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/OpenLogicTypes.h"
#include "DefaultValueHandleLibrary.generated.h"

UCLASS()
class OPENLOGICV2_API UDefaultValueHandleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		static const FText& SetDefaultValue(UPARAM(ref) FOpenLogicDefaultValueHandle Handle);
	
};
