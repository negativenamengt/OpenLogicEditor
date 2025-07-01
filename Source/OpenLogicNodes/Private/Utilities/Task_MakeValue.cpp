/*// Copyright 2024 - NegativeNameSeller

#include "Utilities/Task_MakeValue.h"

#include "Utilities/Task_LogString.h"
#include "NodeLibraryTags.h"
#include "Classes/Properties/OpenLogicAny.h"
#include "Classes/Properties/OpenLogicString.h"
#include "Widgets/NodeBase.h"

UTask_MakeValue::UTask_MakeValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TaskData.Name = "Make (Value)";
	TaskData.Description = FText::FromString("Returns the value of the specified property type.");
	TaskData.Category = "Utilities";
	TaskData.Library = FGameplayTagContainer(TAG_OpenLogicUtilityLibrary);
}

void UTask_MakeValue::OnTaskActivated_Implementation(UObject* Context, FName PinName)
{
	UE_LOG(LogTemp, Warning, TEXT("Task Make Value activated."));
}

void UTask_MakeValue::OnGraphNodeInitialized_Implementation(UNodeBase* Node)
{
	if (Node->InputPins.IsEmpty())
	{
		FOpenLogicPinData InputPinData;
		InputPinData.PinName = "In Value";
		InputPinData.Role = EPinRole::DataProperty;
		InputPinData.PropertyClass = UOpenLogicAny::StaticClass();
		
		Node->AddInputPin(InputPinData, true);
	}

	if (Node->OutputPins.IsEmpty())
	{
		FOpenLogicPinData OutputPinData;
		OutputPinData.PinName = "Return Value";
		OutputPinData.Role = EPinRole::DataProperty;
		OutputPinData.PropertyClass = UOpenLogicAny::StaticClass();
		
		Node->AddOutputPin(OutputPinData, true);
	}
	
	Super::OnGraphNodeInitialized_Implementation(Node);
}*/