// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonWriter.h"
#include "PayloadObject.generated.h"

UENUM(BlueprintType)
enum class EJsonValueType : uint8
{
	None		UMETA(DisplayName = "None"),
	Null		UMETA(DisplayName = "Null"),
	String		UMETA(DisplayName = "String"),
	Number		UMETA(DisplayName = "Number"),
	Boolean		UMETA(DisplayName = "Boolean"),
	Array		UMETA(DisplayName = "Array"),
	Object		UMETA(DisplayName = "Object")
};

UCLASS()
class OPENLOGICV2_API UPayloadObject : public UObject
{
	GENERATED_BODY()

private:
	TSharedPtr<FJsonValue> JsonValue;
public:
	UFUNCTION(Category = "JSON")
		static UPayloadObject* Parse(const FString& JsonString, bool& Success);
	UFUNCTION(Category = "JSON")
		void Stringify(FString& JsonString, bool& Success);
public:
	template <class PrintPolicy> static bool Stringify(TSharedPtr<FJsonValue> JsonValue, const TSharedRef<TJsonWriter<TCHAR, PrintPolicy>>& JsonWriter);
	static TSharedRef<FJsonValue> ConvertToOriginalJsonValue(FProperty* Property, TSharedPtr<FJsonValue> Value, int32& TotalErrors);
	static bool CreateWildcardValue(UPayloadObject* This, FProperty* Property, void* ValuePtr);
	static TSharedRef<FJsonValue> WildcardToPayload(FProperty* Property, void* ValuePtr);
	static UPayloadObject* CreatePayloadValue(TSharedRef<FJsonValue> Value);
	TSharedPtr<FJsonValue> GetField(TSharedPtr<FJsonValue> Value, FString Pattern, EJsonValueType Filter);
};
