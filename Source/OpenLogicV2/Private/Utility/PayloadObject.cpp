// Copyright 2024 - NegativeNameSeller

#include "Utility/PayloadObject.h"
#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Utility/OpenLogicUtility.h"
#include "Runtime/Launch/Resources/Version.h"
#include "InputCoreTypes.h"

UPayloadObject* UPayloadObject::CreatePayloadValue(TSharedRef<FJsonValue> Value)
{
	UPayloadObject* Node = NewObject<UPayloadObject>();
	Node->JsonValue = Value;
	return Node;
}

bool UPayloadObject::CreateWildcardValue(UPayloadObject* This, FProperty* Property, void* ValuePtr)
{
	bool Success = false;

	if (!ValuePtr || !Property || !This)
	{
		return Success;
	}

	int32 TotalErrors = 0;
	// convert clean naming to property naming with hash for blueprints
	const TSharedRef<FJsonValue> InJsonValue = UPayloadObject::ConvertToOriginalJsonValue(Property, This->JsonValue, TotalErrors);

	if (InJsonValue->Type == EJson::Null || InJsonValue->Type == EJson::None)
	{
		return Success;
	}

	Success = UOpenLogicUtility::PayloadToProperty(InJsonValue, Property, ValuePtr);

	Success &= (TotalErrors == 0);

	return Success;
}

TSharedPtr<FJsonValue> UPayloadObject::GetField(TSharedPtr<FJsonValue> Value, FString Pattern, EJsonValueType Filter)
{
	FString Start = Pattern;
	FString Rest;
	Pattern.Split("/", &Start, &Rest);
	if (Value->Type == EJson::Object)
	{
		const TSharedPtr<FJsonObject> Obj = Value->AsObject();
		if (TSharedPtr<FJsonValue>* Res = Obj->Values.Find(Start))
		{
			if (Rest.IsEmpty() && (Filter == EJsonValueType::None || Filter == static_cast<EJsonValueType>((*Res)->Type)))
			{
				return *Res;
			}
			else
			{
				return this->GetField(*Res, Rest, Filter);
			}
		}
	}
	else if (Value->Type == EJson::Array)
	{
		if (Start.IsNumeric())
		{
			TArray<TSharedPtr<FJsonValue>> Arr = Value->AsArray();
			if (const int32 Idx = FCString::Atoi(*Start); Idx > -1 && Idx < Arr.Num())
			{
				if (Rest.IsEmpty() && (Filter == EJsonValueType::None || Filter == static_cast<EJsonValueType>((Arr[Idx])->Type)))
				{
					return Arr[Idx];
				}
				else
				{
					return this->GetField(Arr[Idx], Rest, Filter);
				}
			}
		}
	}
	return nullptr;
}

UPayloadObject* UPayloadObject::Parse(const FString& JsonString, bool& Success)
{
	Success = false;

	const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	TSharedPtr<FJsonValue> JsonValue;

	if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
	{
		Success = true;
		return UPayloadObject::CreatePayloadValue(JsonValue.ToSharedRef());
	}
	else
	{
		return nullptr;
	}
}


TSharedRef<FJsonValue> UPayloadObject::ConvertToOriginalJsonValue(FProperty* Property, TSharedPtr<FJsonValue> Value, int32& Errors)
{
	if (!Value.IsValid() || Property == nullptr || Value->IsNull())
	{
		return MakeShareable(new FJsonValueNull());
	}
	else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> OutArray;
		if (Value->Type == EJson::Array)
		{
			TArray<TSharedPtr<FJsonValue>> InArray = Value->AsArray();
			for (int32 ArrayIndex = 0; ArrayIndex < InArray.Num(); ArrayIndex++)
			{
				OutArray.Add(UPayloadObject::ConvertToOriginalJsonValue(ArrayProperty->Inner, InArray[ArrayIndex], Errors));
			}
		}
		else { Errors++; }
		return MakeShareable(new FJsonValueArray(OutArray));
	}
	else if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> OutSet;
		if (Value->Type == EJson::Array)
		{
			TArray<TSharedPtr<FJsonValue>> InSet = Value->AsArray();
			for (int32 ArrayIndex = 0; ArrayIndex < InSet.Num(); ArrayIndex++)
			{
				OutSet.Add(UPayloadObject::ConvertToOriginalJsonValue(SetProperty->ElementProp, InSet[ArrayIndex], Errors));
			}
		}
		else { Errors++; }
		return MakeShareable(new FJsonValueArray(OutSet));
	}
	else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutMap = MakeShared<FJsonObject>();
		if (Value->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> InMap = Value->AsObject();
			for (TTuple<FString, TSharedPtr<FJsonValue>>& Val : InMap->Values)
			{
				OutMap->SetField(Val.Key, UPayloadObject::ConvertToOriginalJsonValue(MapProperty->ValueProp, Val.Value, Errors));
			}
		}
		else { Errors++; }
		return MakeShareable(new FJsonValueObject(OutMap));
	}
	else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
		if (Value->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> InObject = Value->AsObject();
			for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
			{
				FProperty* Prop = *It;
				if (InObject->HasField(Prop->GetAuthoredName()))
				{
					#if ENGINE_MAJOR_VERSION < 5
					OutObject->SetField(Prop->GetName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetAuthoredName())->ToSharedRef(), Errors));
					#else
					OutObject->SetField(Prop->GetAuthoredName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetAuthoredName())->ToSharedRef(), Errors));
					#endif
				}
				else if (InObject->HasField(Prop->GetName()))
				{
					#if ENGINE_MAJOR_VERSION < 5
					OutObject->SetField(Prop->GetName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetName())->ToSharedRef(), Errors));
					#else
					OutObject->SetField(Prop->GetAuthoredName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetName())->ToSharedRef(), Errors));
					#endif
				}
			}
		}
		else { Errors++; }
		return MakeShareable(new FJsonValueObject(OutObject));
	}
	else if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
		if (ObjectProperty->PropertyClass->IsNative())
		{
			return Value.ToSharedRef();
		}
		if (Value->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> InObject = Value->AsObject();
			for (TFieldIterator<FProperty> It(ObjectProperty->PropertyClass); It; ++It)
			{
				FProperty* Prop = *It;
				if (InObject->HasField(Prop->GetAuthoredName()))
				{
					#if ENGINE_MAJOR_VERSION < 5
					OutObject->SetField(Prop->GetName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetAuthoredName())->ToSharedRef(), Errors));
					#else
					OutObject->SetField(Prop->GetAuthoredName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetAuthoredName())->ToSharedRef(), Errors));
					#endif
				}
				else if (InObject->HasField(Prop->GetName()))
				{
					#if ENGINE_MAJOR_VERSION < 5
					OutObject->SetField(Prop->GetName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetName())->ToSharedRef(), Errors));
					#else
					OutObject->SetField(Prop->GetAuthoredName(), UPayloadObject::ConvertToOriginalJsonValue(Prop, InObject->Values.Find(Prop->GetName())->ToSharedRef(), Errors));
					#endif
				}
			}
			OutObject->SetField(TEXT("_ClassName"), MakeShared<FJsonValueString>(TEXT("")));
		}
		else { Errors++; }
		return MakeShareable(new FJsonValueObject(OutObject));
	}
	else
	{
		return Value.ToSharedRef();
	}
}

TSharedRef<FJsonValue> UPayloadObject::WildcardToPayload(FProperty* Property, void* ValuePtr)
{
	if (ValuePtr == nullptr || Property == nullptr)
	{
		return MakeShareable(new FJsonValueNull());
	}
	else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		FScriptArrayHelper Helper = FScriptArrayHelper::CreateHelperFormInnerProperty(ArrayProperty->Inner, ValuePtr);
		for (int32 ArrayIndex = 0; ArrayIndex < Helper.Num(); ArrayIndex++)
		{
			Array.Add(UPayloadObject::WildcardToPayload(ArrayProperty->Inner, Helper.GetRawPtr(ArrayIndex)));
		}
		return MakeShareable(new FJsonValueArray(Array));
	}
	else if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		FScriptSetHelper Helper = FScriptSetHelper::CreateHelperFormElementProperty(SetProperty->ElementProp, ValuePtr);
		for (int32 ArrayIndex = 0; ArrayIndex < Helper.Num(); ++ArrayIndex)
		{
			Array.Add(UPayloadObject::WildcardToPayload(SetProperty->ElementProp, Helper.GetElementPtr(ArrayIndex)));
		}
		return MakeShareable(new FJsonValueArray(Array));
	}
	else if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		FScriptMapHelper Helper = FScriptMapHelper::CreateHelperFormInnerProperties(MapProperty->KeyProp, MapProperty->ValueProp, ValuePtr);
		for (int32 ArrayIndex = 0; ArrayIndex < Helper.Num(); ++ArrayIndex)
		{
			FString KeyStr;
			TSharedRef<FJsonValue> Key = UPayloadObject::WildcardToPayload(MapProperty->KeyProp, Helper.GetKeyPtr(ArrayIndex));
			if (!Key->TryGetString(KeyStr))
			{
				MapProperty->KeyProp->ExportTextItem_Direct(KeyStr, Helper.GetKeyPtr(ArrayIndex), nullptr, nullptr, 0);
			}
			TSharedRef<FJsonValue> Val = UPayloadObject::WildcardToPayload(MapProperty->ValueProp, Helper.GetValuePtr(ArrayIndex));
			JsonObject->SetField(KeyStr, Val);
		}
		return MakeShareable(new FJsonValueObject(JsonObject));
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			FProperty* Prop = *It;
			JsonObject->SetField(Prop->GetAuthoredName(), UPayloadObject::WildcardToPayload(Prop, Prop->ContainerPtrToValuePtr<void*>(ValuePtr)));
		}
		return MakeShareable(new FJsonValueObject(JsonObject));
	}

	else if (FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
	{
		void* PropValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
		if (PropValue == nullptr)
		{
			return MakeShareable(new FJsonValueNull());
		}
		if (ObjectProperty->PropertyClass->IsNative())
		{
			TSharedPtr<FJsonValue> Value = FJsonObjectConverter::UPropertyToJsonValue(Property, ValuePtr, 0, 0);
			return Value.ToSharedRef();
		}
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		for (TFieldIterator<FProperty> It(ObjectProperty->PropertyClass); It; ++It)
		{
			FProperty* Prop = *It;
			JsonObject->SetField(Prop->GetAuthoredName(), UPayloadObject::WildcardToPayload(Prop, Prop->ContainerPtrToValuePtr<void*>(PropValue)));
		}
		return MakeShareable(new FJsonValueObject(JsonObject));
	}
	else
	{
		TSharedPtr<FJsonValue> Value = FJsonObjectConverter::UPropertyToJsonValue(Property, ValuePtr, 0, 0);
		return Value.ToSharedRef();
	}
}

template<class PrintPolicy>
bool UPayloadObject::Stringify(TSharedPtr<FJsonValue> JsonValue, const TSharedRef<TJsonWriter<TCHAR, PrintPolicy>>& JsonWriter)
{
	if (!JsonValue.IsValid())
	{
		return false;
	}

	bool Result = false;

	// Check if the JsonValue is a primitive type
	if (JsonValue->Type == EJson::String || JsonValue->Type == EJson::Number ||
		JsonValue->Type == EJson::Boolean || JsonValue->Type == EJson::Null)
	{
		TArray<TSharedPtr<FJsonValue>> ValueArray;
		ValueArray.Add(JsonValue);
		Result = FJsonSerializer::Serialize(ValueArray, JsonWriter);
	}
	else
	{
		switch (JsonValue->Type)
		{
		case EJson::Array:
			Result = FJsonSerializer::Serialize(JsonValue->AsArray(), JsonWriter);
			break;
		case EJson::Object:
			Result = FJsonSerializer::Serialize(JsonValue->AsObject().ToSharedRef(), JsonWriter);
			break;
		default:
			break;
		}
	}

	JsonWriter->Close();
	return Result;
}


void UPayloadObject::Stringify(FString& Payload, bool& Success)
{
	Success = false;
	Payload = "";
	Success = UPayloadObject::Stringify<TCondensedJsonPrintPolicy<TCHAR>>(
		this->JsonValue,
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Payload, 0)
	);
}