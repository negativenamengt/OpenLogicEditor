// Copyright 2024 - NegativeNameSeller

#include "Customization/PropertyTypeCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "Core/OpenLogicTypes.h"
#include "Tasks/OpenLogicProperty.h"
#include "Tasks/OpenLogicTask.h"

TSharedRef<IPropertyTypeCustomization> FPinDataCustomization::MakeInstance()
{
	return MakeShareable(new FPinDataCustomization);
}

void FPinDataCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Retrieve whether the member is a output pin
	TSharedPtr<IPropertyHandle> PinDisplayNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PinName));
	TSharedPtr<IPropertyHandle> PinRoleHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, Role));
	TSharedPtr<IPropertyHandle> PinPropertyClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PropertyClass));

	if (FName PinName; PinDisplayNameHandle->GetValue(PinName) == FPropertyAccess::Success && PinName.IsNone())
	{
		uint32 ArrayCount = 0;
		PropertyHandle->GetParentHandle()->GetNumChildren(ArrayCount);

		const FString& Prefix = GetIsOutputPin(PropertyHandle) ? "then" : "execute";

		if (ArrayCount > 1)
		{
			const FString& GeneratedPinName = FString::Printf(TEXT("%s %d"), *Prefix, ArrayCount);
			PinDisplayNameHandle->SetValue(FName(*GeneratedPinName));	
		} else
		{
			PinDisplayNameHandle->SetValue(FName(*Prefix));
		}
	}
	
	HeaderRow.NameContent()
	[
		SNew(SBox)
		.WidthOverride(150.0f)
		[
			PinDisplayNameHandle->CreatePropertyValueWidget(false)
		]
	];

	HeaderRow.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.Padding(0.0f, 0.0f, 5.0f, 0.0f)
			.WidthOverride(150.0f)
			[
				PinRoleHandle->CreatePropertyValueWidget(false)
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.Visibility(this, &FPinDataCustomization::GetPinPropertyVisibility, PinRoleHandle)
			.WidthOverride(150.0f)
			[
				PinPropertyClassHandle->CreatePropertyValueWidget(false)
			]
		]
	];
}

void FPinDataCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Retrieve the pin data
	FOpenLogicPinData* PinData = GetPinDataPtr(PropertyHandle);
	if (!PinData)
	{
		return;
	}

	bool bIsOutputPin = false;

	if (PropertyHandle->GetParentHandle())
	{
		bIsOutputPin = GetIsOutputPin(PropertyHandle->GetParentHandle().ToSharedRef());
	}
	
	TSharedPtr<IPropertyUtilities> PropertyUtilities = CustomizationUtils.GetPropertyUtilities();

	TSharedPtr<IPropertyHandle> PinDisplayNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PinName));
	
	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		if (!ChildHandle->IsValidHandle())
		{
			continue;
		}

		// Detect when a property value has changed
		ChildHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([ChildHandle, PropertyUtilities, PropertyHandle]
		{
			const FName& PropertyName = ChildHandle->GetProperty()->GetFName();

			if (PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, Role))
			{
				// Clear the property class
				TSharedPtr<IPropertyHandle> PropertyClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PropertyClass));
				if (PropertyClassHandle.IsValid())
				{
					PropertyClassHandle->ResetToDefault();
				}
			}
			
			if (PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, Role) || PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PropertyClass))
			{
				// Clear the default value
				TSharedPtr<IPropertyHandle> DefaultValueHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, DefaultValue));
				if (DefaultValueHandle.IsValid())
				{
					DefaultValueHandle->ResetToDefault();
				}

				if (PropertyUtilities.IsValid())
				{
					PropertyUtilities->ForceRefresh();
				}
			}
		}));
		
		const FName PropertyName = ChildHandle->GetProperty()->GetFName();
		
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, Role) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PropertyClass) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, PinName) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, DefaultValue) && (bIsOutputPin || PinData->Role != EPinRole::DataProperty))
		{
			continue;
		}

		if (PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, Attributes))
		{
			if (bIsOutputPin)
			{
				continue;
			}
			
			for (const FName& AttributeName : PinData->GetRelevantPropertyAttributes())
			{
				TSharedPtr<IPropertyHandle> AttributeHandle = ChildHandle->GetChildHandle(AttributeName);
				if (!AttributeHandle.IsValid())
				{
					continue;
				}

				ChildBuilder.AddProperty(AttributeHandle.ToSharedRef());
			}

			continue;
		}

		if (PropertyName == GET_MEMBER_NAME_CHECKED(FOpenLogicPinData, DefaultValue))
		{
			if (PinData->Role != EPinRole::DataProperty)
			{
				continue;
			}

			UOpenLogicProperty* Property = PinData->PropertyClass.GetDefaultObject();
			if (!Property)
			{
				continue;
			}

			TSharedPtr<FOpenLogicDefaultValueHandle> DefaultValueHandle = MakeShared<FOpenLogicDefaultValueHandle>();
			DefaultValueHandle->SetPropertyHandle(ChildHandle);
			
			TArray<TSharedRef<SWidget>> InputWidgets = Property->CreateEditorInputWidgets(DefaultValueHandle.ToSharedRef(), PinData->Attributes);
			if (InputWidgets.Num() == 0)
			{
				ChildBuilder.AddProperty(ChildHandle);
				continue;
			}

			// Add a group for the default value to handle multiple row entries		
			IDetailGroup& DefaultValueGroup = ChildBuilder.AddGroup(TEXT("Default Value"), FText::FromString("Default Value"));
		
			if (InputWidgets.Num() == 1)
			{
				DefaultValueGroup.HeaderRow()
					.NameContent()
					[
						ChildHandle->CreatePropertyNameWidget()
					]
					.ValueContent()
					[
						InputWidgets[0]
					];
			} else
			{
				for (TSharedRef<SWidget> InputWidget : InputWidgets)
				{
					DefaultValueGroup.AddWidgetRow()
						.NameContent()
						[
							ChildHandle->CreatePropertyNameWidget(FText::FromName(InputWidget->GetTag()))
						]
						.ValueContent()
						[
							InputWidget
						];
				}
			}

			continue;
		}
		
		ChildBuilder.AddProperty(ChildHandle);
	}
}

FOpenLogicPinData* FPinDataCustomization::GetPinDataPtr(TSharedRef<IPropertyHandle> PropertyHandle) const
{
    void* PinDataPtr;
    if (PropertyHandle->GetValueData(PinDataPtr) != FPropertyAccess::Success)
    {
        return nullptr;
    }

    return static_cast<FOpenLogicPinData*>(PinDataPtr);
}

bool FPinDataCustomization::GetIsOutputPin(TSharedRef<IPropertyHandle> PropertyHandle) const
{
	FName PropertyName = PropertyHandle->GetProperty()->GetFName();
	return PropertyName == "OutputPins";
}

EVisibility FPinDataCustomization::GetPinPropertyVisibility(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	uint8 RoleValue;
	if (PropertyHandle->GetValue(RoleValue) != FPropertyAccess::Success)
	{
		return EVisibility::Visible;
	}

	return static_cast<EPinRole>(RoleValue) == EPinRole::DataProperty ? EVisibility::Visible : EVisibility::Collapsed;
}
