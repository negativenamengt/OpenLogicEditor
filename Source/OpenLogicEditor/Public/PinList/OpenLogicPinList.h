// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphPin.h"

class OPENLOGICEDITOR_API SOpenLogicPinList : public SGraphPin
{
public:
	virtual ~SOpenLogicPinList() override { Blueprint = nullptr; }
	
public:
	SLATE_BEGIN_ARGS(SOpenLogicPinList) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
	void RefreshOptions(UEdGraphPin* InPin);

	TSharedPtr<FName> SelectedOption;
	
protected:
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
	void ParseDefaultValue();
	void GetValueAsPinName(FName& OutPinName) const;
	bool IsGraphPinValid() const;
	void OnPinComboBoxSelectionChanged(TSharedPtr<FName> ItemSelected, ESelectInfo::Type SelectInfo);
	UBlueprint* Blueprint = nullptr;
	TSharedPtr<class SNameComboBox> PinComboBox;
	TArray<TSharedPtr<FName>> Options;
	TMap<FName, FName> OldNameToNewNameMap;
};

class OPENLOGICEDITOR_API FOpenLogicPinListFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override;
};