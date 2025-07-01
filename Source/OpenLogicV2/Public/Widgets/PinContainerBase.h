// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinContainerBase.generated.h"

UENUM(BlueprintType)
enum class EOpenLogicPinContainerType : uint8
{
	Vertical,
	Horizontal
};

class UExecutionPinBase;

UCLASS(Deprecated)
class OPENLOGICV2_API UDEPRECATED_PinContainerBase : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenLogic|Properties")
		EOpenLogicPinContainerType Type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OpenLogic|Properties")
		TSubclassOf<UExecutionPinBase> PinClass;

public:
	// Called when a pin is added to the container.
	UFUNCTION(BlueprintNativeEvent)
		void OnPinAdded(UExecutionPinBase* Pin);

	// Called when a pin is removed from the container.
	UFUNCTION(BlueprintNativeEvent)
		void OnPinRemoved();

};
