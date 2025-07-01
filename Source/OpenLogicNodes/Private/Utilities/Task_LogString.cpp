// Copyright 2024 - NegativeNameSeller

#include "Utilities/Task_LogString.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicString.h"

UTask_LogString::UTask_LogString(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Log String";
	TaskData.Description = FText::FromString("If Condition is true, 'True' will be executed, otherwise 'False'.");
	TaskData.Category = "Flow Control";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicFlowControlLibrary);

	// Input pins
	TaskData.InputPins.Add(FOpenLogicPinData("execute"));
	TaskData.InputPins.Add(FOpenLogicPinData("In String", FText::GetEmpty(), EPinRole::DataProperty, UOpenLogicString::StaticClass()));

	// Output pins
	TaskData.OutputPins.Add(FOpenLogicPinData("then"));
}

void UTask_LogString::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	FString StringValue = GetPropertyValueByAttribute<FString>(FName("In String"));
	UE_LOG(LogTemp, Log, TEXT("Log String: %s"), *StringValue);

	CompleteTask("then");
}
