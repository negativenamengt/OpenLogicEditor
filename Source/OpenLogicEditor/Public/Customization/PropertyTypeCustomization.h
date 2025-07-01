// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"

class UOpenLogicTask;
class UOpenLogicProperty;
struct FOpenLogicPinData;

class FPinDataCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	EVisibility GetPinPropertyVisibility(TSharedPtr<IPropertyHandle> PropertyHandle) const;
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	FOpenLogicPinData* GetPinDataPtr(TSharedRef<IPropertyHandle> PropertyHandle) const;
	bool GetIsOutputPin(TSharedRef<IPropertyHandle> PropertyHandle) const;
};