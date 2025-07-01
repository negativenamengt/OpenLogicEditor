// Fill out your copyright notice in the Description page of Project Settings.


#include "PinList/OpenLogicPinList.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "SNameComboBox.h"
#include "Core/OpenLogicTypes.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Tasks/OpenLogicTask.h"

TSharedPtr<SGraphPin> FOpenLogicPinListFactory::CreatePin(UEdGraphPin* Pin) const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	if (Pin->PinType.PinCategory != K2Schema->PC_Struct)
	{
		return nullptr;
	}

	UStruct* SubCategoryStruct = Cast<UStruct>(Pin->PinType.PinSubCategoryObject);
	if (!SubCategoryStruct || !SubCategoryStruct->IsChildOf(FOpenLogicPinHandle::StaticStruct()))
	{
		return nullptr;
	}
	
	return SNew(SOpenLogicPinList, Pin);
}

void SOpenLogicPinList::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(InPin->GetOwningNode()->GetGraph());

	RefreshOptions(InPin);
	SGraphPin::Construct(SGraphPin::FArguments(), InPin);
}

void SOpenLogicPinList::RefreshOptions(UEdGraphPin* InPin)
{
	Options.Empty();

	if (!Blueprint || !Blueprint->GeneratedClass || !Blueprint->GeneratedClass->IsChildOf(UOpenLogicTask::StaticClass()))
	{
		return;
	}

	UOpenLogicTask* Task = Cast<UOpenLogicTask>(Blueprint->GeneratedClass->ClassDefaultObject);
	UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(InPin->GetOwningNode());
	if (!Task || !CallFunctionNode)
	{
		return;
	}

	UFunction* Function = Blueprint->GeneratedClass->FindFunctionByName(CallFunctionNode->FunctionReference.GetMemberName());
	if (!Function)
	{
		return;
	}
	
	EOpenLogicPinDirection Direction = EOpenLogicPinDirection::Input;

	if (Function->GetMetaData("PinDirection") == "Output")
	{
		Direction = EOpenLogicPinDirection::Output;
	}

	EPinRole Role = EPinRole::FlowControl;

	if (Function->GetMetaData("PinRole") == "DataProperty")
	{
		Role = EPinRole::DataProperty;
	}
	
	// Retrieve the pin names based on the information above
	for (const FOpenLogicPinData PinData : Direction == EOpenLogicPinDirection::Input ? Task->TaskData.InputPins : Task->TaskData.OutputPins)
    {
		if (PinData.PinName.IsNone())
			continue;

		if (PinData.Role != Role)
			continue;
		
        Options.Add(MakeShareable(new FName(PinData.PinName)));
    }
}

TSharedRef<SWidget> SOpenLogicPinList::GetDefaultValueWidget()
{
	ParseDefaultValue();
	
	return SAssignNew(PinComboBox, SNameComboBox)
		.OptionsSource(&Options)
		.InitiallySelectedItem(SelectedOption)
		.OnSelectionChanged(this, &SOpenLogicPinList::OnPinComboBoxSelectionChanged)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility);
}

void SOpenLogicPinList::ParseDefaultValue()
{
	FName PinName;
	GetValueAsPinName(PinName);

	for (const TSharedPtr<FName>& Option : Options)
	{
		if (*Option == PinName)
		{
			SelectedOption = Option;
			break;
		}
	}
}

void SOpenLogicPinList::GetValueAsPinName(FName& OutPinName) const
{
	if (!IsGraphPinValid())
	{
		return;
	}

	FString Value = GraphPinObj->GetDefaultAsString();
	if (Value.IsEmpty() || !Value.StartsWith(TEXT("(PinName=\"")) || !Value.EndsWith(TEXT("\")")))
	{
		return;
	}

	Value.RemoveFromStart(TEXT("(PinName=\""));
	Value.RemoveFromEnd(TEXT("\")"));

	OutPinName = FName(*Value);
}

bool SOpenLogicPinList::IsGraphPinValid() const
{
	return GraphPinObj && GraphPinObj->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && Cast<UStruct>(GraphPinObj->PinType.PinSubCategoryObject)->IsChildOf(FOpenLogicPinHandle::StaticStruct());
}

void SOpenLogicPinList::OnPinComboBoxSelectionChanged(TSharedPtr<FName> ItemSelected, ESelectInfo::Type SelectInfo)
{
	if (!GraphPinObj)
	{
		return;
	}

	const FName SelectedPinName = ItemSelected.IsValid() ? *ItemSelected : NAME_None;
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();

	// Format the value into a struct string
	const FString ValueToStore = TEXT("(PinName=\"") + SelectedPinName.ToString() + TEXT("\")");

	// Transaction for undo/redo
	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GraphEd_SetPinValue", "Set Pin Value"));
	GraphPinObj->Modify();

	// Set the value
	Schema->TrySetDefaultValue(*GraphPinObj, ValueToStore);
}