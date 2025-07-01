// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Core/OpenLogicTypes.h"
#include "OpenLogicProperty.generated.h"

class UProperty;
class UExecutionPinBase;

UENUM(BlueprintType)
enum class EOpenLogicUnderlyingType : uint8
{
	Boolean,
	Byte,
	Int,
	Float,
	Double,
	String,
	Name,
	Text,
	Object,
	Struct,
	Enum,
	Class,
	Wildcard
};

UCLASS(Blueprintable, Abstract)
class OPENLOGICV2_API UOpenLogicProperty : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = OpenLogic)
		FText PropertyDisplayName = FText();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = OpenLogic)
		FLinearColor PropertyColor = FLinearColor::White;

	// Whether this property can connect to multiple input properties.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = OpenLogic)
		bool CanConnectToMultiple = true;

	// Determines whether this property resolves its type dynamically. (Wildcard)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = OpenLogic)
		bool bResolvesTypeDynamically = false;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic", meta = (Bitmask = "", BitmaskEnum = "/Script/OpenLogicV2.EOpenLogicPinAttribute"))
		uint8 SupportedAttributes;

	// The FProperty class that this property is based on.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic")
		EOpenLogicUnderlyingType UnderlyingType = EOpenLogicUnderlyingType::Wildcard;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OpenLogic", meta = (EditCondition = "UnderlyingType == EOpenLogicUnderlyingType::Struct"))
		TObjectPtr<UScriptStruct> StructType = nullptr;

public:
	// Only available in editor for setting default values in FOpenLogicPinData in a more user-friendly way.
	virtual TArray<TSharedRef<SWidget>> CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const { return {}; }

	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		FOpenLogicPinValueDisplay CreatePinDefaultValueWidget() const;
	
public:
	// This function calls IsCompatibleWith() with both properties to find a possible connection.
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		bool CanConnectTo(UOpenLogicProperty* OtherProperty);

public:
	UFUNCTION(BlueprintNativeEvent, Category = "OpenLogic")
		bool IsCompatibleWith(UOpenLogicProperty* OtherProperty);

	bool ValidatePropertyType(FProperty* Property) const;
};

UINTERFACE(Blueprintable)
class OPENLOGICV2_API UOpenLogicPropertyWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class OPENLOGICV2_API IOpenLogicPropertyWidgetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = OpenLogic)
		void ReceiveDefaultValueAsString(const FString& InDefaultValue);
};