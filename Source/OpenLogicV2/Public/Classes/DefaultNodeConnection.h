// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "Classes/CustomConnection.h"
#include "Types/SlateEnums.h"
#include "DefaultNodeConnection.generated.h"

UCLASS()
class OPENLOGICV2_API UDefaultNodeConnection : public UCustomConnection
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		float TangentScaleFactor = 210.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		float PinOffset = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		FVector2D PositionOffset = FVector2D::Zero();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		float Thickness = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		EWireDrawingStyle DrawingStyle = EWireDrawingStyle::Spline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		TEnumAsByte<EOrientation> DrawingOrientation = EOrientation::Orient_Horizontal;

public:
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

private:
	UFUNCTION()
		void RecacheValues();

	UFUNCTION()
		void RecachePreviewValues();

private:
	UPROPERTY()
		FCachedWireData CachedWireData;

protected:
	virtual void OnPaint_Implementation(UPARAM(ref) FPaintContext& Context) override;
	virtual void OnRecalculateConnection_Implementation() override;

protected:
	UFUNCTION()
		void DrawSpline(UPARAM(ref) FPaintContext& Context);

	UFUNCTION()
		void DrawInterpolatedPoints(UPARAM(ref) FPaintContext& Context);

private:
	UFUNCTION()
		void CacheInterpolation();

private:
	UFUNCTION()
		float GetPinOffset();

private:
	UFUNCTION()
		const bool IsDrawingHorizontal() const;

	UFUNCTION()
		const bool IsDrawingVertical() const;

private:
	// Returns the position offset for the source pin
	UFUNCTION()
		FVector2D GetSourcePinPositionOffset();

	// Returns the position offset for the target pin
	UFUNCTION()
		FVector2D GetTargetPinPositionOffset();

private:
	// Calculates the tangent vector used for spline wires
	UFUNCTION()
		FVector2D CalculateDirectionalTangent();

	// Calculates the points used for subway wires
	UFUNCTION()
		TArray<FVector2D> CalculateSubwayPoints();
};
