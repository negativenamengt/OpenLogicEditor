// Copyright 2024 - NegativeNameSeller

#include "Utility/OpenLogicUtility.h"
#include "JsonObjectConverter.h"

FText UOpenLogicUtility::GetPinValidityMessage(EPinConnectionValidity ConnectionValidity, UExecutionPinBase* SourcePin, UExecutionPinBase* TargetPin)
{
	if (!SourcePin || !TargetPin)
	{
		return FText::GetEmpty();
	}

	FText SourcePinProperty = SourcePin->GetPropertyDefaultObject() ? SourcePin->GetPropertyDefaultObject()->PropertyDisplayName : FText::FromString("Exec");
	FText TargetPinProperty = TargetPin->GetPropertyDefaultObject() ? TargetPin->GetPropertyDefaultObject()->PropertyDisplayName : FText::FromString("Exec");

	switch (ConnectionValidity)
	{
	case EPinConnectionValidity::None:
		return FText::GetEmpty();

	case EPinConnectionValidity::IdenticalPin:
		return FText::FromString("Cannot connect a pin to itself.");

	case EPinConnectionValidity::IdenticalNode:
		return FText::FromString("Both are on the same node.");

	case EPinConnectionValidity::DirectionConflict:
		return FText::FromString("Directions are not compatible.");

	case EPinConnectionValidity::PropertyMismatch:
	case EPinConnectionValidity::RoleMismatch:
		return FText::FromString(FString::Printf(TEXT("%s is not compatible with %s."), *SourcePinProperty.ToString(), *TargetPinProperty.ToString()));

	case EPinConnectionValidity::Replaceable:
		return FText::FromString("Replace existing output connections.");

	case EPinConnectionValidity::Compatible:
		return FText::GetEmpty();

	default:
		return FText::GetEmpty();
	}
}

UOpenLogicRuntimeGraph* UOpenLogicUtility::CreateRuntimeGraphFromObject(UObject* Outer, UObject* ContextObject, UOpenLogicGraph* GraphObject)
{
	if (!IsValid(GraphObject))
	{
		UE_LOG(OpenLogicLog, Warning, TEXT("UOpenLogicUtility::CreateRuntimeWorkerFromObject: can not create a runtime worker from an invalid graph object."))
		return nullptr;
	}

	return CreateRuntimeGraphFromStruct(Outer, ContextObject, GraphObject->GraphData);
}

UOpenLogicRuntimeGraph* UOpenLogicUtility::CreateRuntimeGraphFromStruct(UObject* Outer, UObject* ContextObject, FOpenLogicGraphData GraphData)
{
	if (!Outer)
	{
		Outer = ContextObject;
	}
	
	UOpenLogicRuntimeGraph* NewRuntimeGraph = NewObject<UOpenLogicRuntimeGraph>(Outer, UOpenLogicRuntimeGraph::StaticClass());
	NewRuntimeGraph->SetGraphData(GraphData);
	NewRuntimeGraph->SetContext(ContextObject);

	return NewRuntimeGraph;
}

void UOpenLogicUtility::LagTest()
{
	// Log the start of the test
	UE_LOG(LogTemp, Warning, TEXT("LagTest started"));

	int32 PrimeCount = 0;

	// Perform a heavy, CPU-intensive task that causes lag (finding prime numbers)
	for (int32 Number = 2; Number <= 500000; Number++)
	{
		bool bIsPrime = true;

		// Check if the number is a prime number
		for (int32 i = 2; i < Number; i++)
		{
			if (Number % i == 0)
			{
				bIsPrime = false;
				break;
			}
		}

		// If the number is prime, increment the prime count
		if (bIsPrime)
		{
			PrimeCount++;
		}
	}

	// Log the result and completion of the test
	UE_LOG(LogTemp, Warning, TEXT("LagTest completed. Prime Count: %d"), PrimeCount);
	
}

bool UOpenLogicUtility::PayloadToProperty(TSharedPtr<FJsonValue> Payload, FProperty* Property, void* ValuePtr)
{
	if (!Payload.IsValid() || !Property || !ValuePtr) return false;

	if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		FScriptSetHelper Helper = FScriptSetHelper::CreateHelperFormElementProperty(SetProperty->ElementProp, ValuePtr);
		auto OutJsonArray = Payload->AsArray();
		Helper.EmptyElements(OutJsonArray.Num());

		for (int32 i = 0; i < OutJsonArray.Num(); ++i)
		{
			int32 AddIdx = Helper.AddDefaultValue_Invalid_NeedsRehash();
			if (!PayloadToProperty(OutJsonArray[i], SetProperty->ElementProp, Helper.GetElementPtr(AddIdx)))
				return false;
		}
		Helper.Rehash();
		return true;
	}
	else if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		FScriptArrayHelper Helper = FScriptArrayHelper::CreateHelperFormInnerProperty(ArrayProperty->Inner, ValuePtr);
		auto OutJsonArray = Payload->AsArray();
		Helper.EmptyAndAddValues(OutJsonArray.Num());

		for (int32 i = 0; i < OutJsonArray.Num(); i++)
		{
			if (!PayloadToProperty(OutJsonArray[i], ArrayProperty->Inner, Helper.GetRawPtr(i)))
				return false;
		}
		return true;
	}
    else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
    {
        FScriptMapHelper Helper = FScriptMapHelper::CreateHelperFormInnerProperties(MapProperty->KeyProp, MapProperty->ValueProp, ValuePtr);

        TSharedPtr<FJsonObject> OutJsonObject = Payload->AsObject();
        TArray<FString> OutKeys;
        OutJsonObject->Values.GetKeys(OutKeys);

		for (int32 ArrayIndex = 0; ArrayIndex < OutJsonObject->Values.Num(); ++ArrayIndex)
		{
			int32 AddIdx = Helper.AddDefaultValue_Invalid_NeedsRehash();
			TSharedPtr<FJsonValue> KeyJsonValue = MakeShareable(new FJsonValueString(OutKeys[ArrayIndex]));
			TSharedPtr<FJsonValue>* ValueJsonValue = OutJsonObject->Values.Find(OutKeys[ArrayIndex]);

			if (FJsonObjectConverter::JsonValueToUProperty(KeyJsonValue, MapProperty->KeyProp, Helper.GetKeyPtr(AddIdx), 0, 0) && PayloadToProperty(*ValueJsonValue, MapProperty->ValueProp, Helper.GetValuePtr(AddIdx)))
			{
				continue;
			}

			return false;
		}

		Helper.Rehash();
        return true;
    }
    else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        TSharedPtr<FJsonObject> OutJsonObject = Payload->AsObject();

        for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
        {
            FProperty* Prop = *It;
            TSharedPtr<FJsonValue> OutJsonValue = OutJsonObject->TryGetField(Prop->GetAuthoredName());
            if (!OutJsonValue.IsValid())
            {
                OutJsonValue = OutJsonObject->TryGetField(Prop->GetName());
            }

            if (OutJsonValue.IsValid() && !UOpenLogicUtility::PayloadToProperty(OutJsonValue, Prop, Prop->ContainerPtrToValuePtr<void*>(ValuePtr)))
            {
                return false;
            }
        }

        return true;
    }
	else if (const FObjectProperty* ObjProperty = CastField<FObjectProperty>(Property))
	{
		if (Payload->Type == EJson::String)
		{
			FString ClassRefString = Payload->AsString();
			UClass* TargetClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassRefString);

			if (!TargetClass)
			{
				return false;
			}

			ObjProperty->SetObjectPropertyValue(ValuePtr, TargetClass);

			return true;
		}
		const UClass* ObjClass = ObjProperty->PropertyClass;
		TSharedPtr<FJsonObject> OutJsonObject = Payload->AsObject();
		UObject* ObjPropValue = ObjProperty->GetObjectPropertyValue(ValuePtr);

		if (!ObjPropValue)
		{
			ObjPropValue = StaticAllocateObject(ObjClass, GetTransientPackage(), NAME_None, EObjectFlags::RF_NoFlags, EInternalObjectFlags::None, false);
			(*ObjClass->ClassConstructor)(FObjectInitializer(ObjPropValue, ObjClass->ClassDefaultObject, EObjectInitializerOptions::None));
			ObjProperty->SetObjectPropertyValue(ValuePtr, ObjPropValue);
		}

		for (TFieldIterator<FProperty> It(ObjClass); It; ++It)
		{
			FProperty* Prop = *It;
			TSharedPtr<FJsonValue> OutPayload = nullptr;
			if (OutJsonObject->HasField(Prop->GetAuthoredName()))
			{
				OutPayload = OutJsonObject->TryGetField(Prop->GetAuthoredName());
			}
			else if (OutJsonObject->HasField(Prop->GetName()))
			{
				OutPayload = OutJsonObject->TryGetField(Prop->GetName());
			}
			if (OutPayload.IsValid() && !UOpenLogicUtility::PayloadToProperty(OutPayload, Prop, Prop->ContainerPtrToValuePtr<void*>(ObjPropValue))) { return false; }
		}
		return true;
	}
	else
	{
		return FJsonObjectConverter::JsonValueToUProperty(Payload, Property, ValuePtr, 0, 0);
	}
}
