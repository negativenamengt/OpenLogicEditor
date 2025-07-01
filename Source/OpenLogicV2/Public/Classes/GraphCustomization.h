// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "GraphCustomization.generated.h"

class UNodeBase;

UCLASS(BlueprintType)
class OPENLOGICV2_API UGraphCustomization : public UDataAsset
{
	GENERATED_BODY()

public:
	UGraphCustomization(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties")
		FLinearColor BackgroundColor = FLinearColor(0.0193, 0.0193, 0.0193, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties")
		bool ShowBackground = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties")
		FSlateBrush SelectionBoxBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties")
		bool ShowNativeGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties", meta = (EditCondition = "!ShowNativeGrid", EditConditionHides))
		UObject* CustomGridBrush;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Properties")
		float GridSnapSize = 0.0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid", EditConditionHides))
		bool ShowRulerGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowRulerGrid", EditConditionHides))
		float RulerGridSpacing = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowRulerGrid", EditConditionHides))
		float RulerGridThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowRulerGrid", EditConditionHides))
		FLinearColor RulerGridColor = FLinearColor(0.008023, 0.008023, 0.008023, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowNativeGrid", EditConditionHides))
		bool ShowMinorGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowMinorGrid", EditConditionHides))
		float MinorGridThickness = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowMinorGrid", EditConditionHides))
		FLinearColor MinorGridColor = FLinearColor(0.0356, 0.0356, 0.0356, 1);
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowNativeGrid", EditConditionHides))
		bool ShowOrigin = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowOrigin", EditConditionHides))
		float OriginThickness = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Native Grid", meta = (EditCondition = "ShowNativeGrid && ShowOrigin", EditConditionHides))
		FLinearColor OriginColor = FLinearColor::Black;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = NodeClasses)
	TSoftClassPtr<UNodeBase> DefaultNodeClass;
};