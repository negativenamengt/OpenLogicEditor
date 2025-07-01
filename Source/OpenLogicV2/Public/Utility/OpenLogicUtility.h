// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tasks/OpenLogicTask.h"
#include "Utility/PayloadObject.h"
#include "Classes/OpenLogicGraph.h"
#include "Runtime/OpenLogicRuntimeGraph.h"
#include "Widgets/ExecutionPinBase.h"
#include "OpenLogicUtility.generated.h"

UCLASS(Blueprintable)
class OPENLOGICV2_API UOpenLogicUtility : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		static FText GetPinValidityMessage(EPinConnectionValidity ConnectionValidity, UExecutionPinBase* SourcePin, UExecutionPinBase* TargetPin);

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (DefaultToSelf = "Outer"))
		static UOpenLogicRuntimeGraph* CreateRuntimeGraphFromObject(UObject* Outer, UObject* ContextObject, UOpenLogicGraph* GraphObject);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (DefaultToSelf = "Outer"))
		static UOpenLogicRuntimeGraph* CreateRuntimeGraphFromStruct(UObject* Outer, UObject* ContextObject, FOpenLogicGraphData GraphData);

	UFUNCTION(BlueprintCallable, Category = "OpenLogic", meta = (DefaultToSelf = "ContextObject"))
		static void LagTest();
public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic", CustomThunk, meta = (CustomStructureParam = "InStruct"))
		void StructToPayload(FString& Payload, bool& Success, const UStruct* InStruct);

	DECLARE_FUNCTION(execStructToPayload)
	{
		P_GET_PROPERTY_REF(FStrProperty, Json);
		P_GET_UBOOL_REF(Success);

		Stack.Step(Stack.Object, NULL);

		FProperty* Prop = Stack.MostRecentProperty;
		void* Ptr = Stack.MostRecentPropertyAddress;

		P_FINISH;

		UPayloadObject* Wrapper = UPayloadObject::CreatePayloadValue(UPayloadObject::WildcardToPayload(Prop, Ptr));
		Wrapper->Stringify(Json, Success);
	}

public:
	static bool PayloadToProperty(TSharedPtr<FJsonValue> Payload, FProperty* Property, void* ValuePtr);
};
