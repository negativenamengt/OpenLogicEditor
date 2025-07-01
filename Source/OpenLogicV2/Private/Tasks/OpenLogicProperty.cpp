// Copyright 2024 - NegativeNameSeller

#include "Tasks/OpenLogicProperty.h"

bool UOpenLogicProperty::IsCompatibleWith_Implementation(UOpenLogicProperty* OtherProperty)
{
	return OtherProperty->GetClass() == GetClass();
}

FOpenLogicPinValueDisplay UOpenLogicProperty::CreatePinDefaultValueWidget_Implementation() const
{
	return FOpenLogicPinValueDisplay();
}

bool UOpenLogicProperty::CanConnectTo(UOpenLogicProperty* OtherProperty)
{
	if (this->IsCompatibleWith(OtherProperty))
	{
		return true;
	}

	if (OtherProperty->IsCompatibleWith(this))
	{
		return true;
	}

	return false;
}

bool UOpenLogicProperty::ValidatePropertyType(FProperty* Property) const
{
	if (!Property)
	{
		return false;
	}

	// Handle wildcard types
	if (UnderlyingType == EOpenLogicUnderlyingType::Wildcard)
	{
		return true;
	}

	switch (UnderlyingType)
	{
	case EOpenLogicUnderlyingType::Boolean:
		return Property->IsA<FBoolProperty>();
	case EOpenLogicUnderlyingType::Byte:
		return Property->IsA<FByteProperty>();
	case EOpenLogicUnderlyingType::Int:
		return Property->IsA<FIntProperty>();
	case EOpenLogicUnderlyingType::Float:
		return Property->IsA<FFloatProperty>();
	case EOpenLogicUnderlyingType::Double:
		return Property->IsA<FDoubleProperty>();
	case EOpenLogicUnderlyingType::String:
		return Property->IsA<FStrProperty>();
	case EOpenLogicUnderlyingType::Name:
		return Property->IsA<FNameProperty>();
	case EOpenLogicUnderlyingType::Text:
		return Property->IsA<FTextProperty>();
	case EOpenLogicUnderlyingType::Enum:
		return Property->IsA<FEnumProperty>() || Property->IsA<FByteProperty>();
	case EOpenLogicUnderlyingType::Object:
		return Property->IsA<FObjectPropertyBase>();
	case EOpenLogicUnderlyingType::Class:
		return Property->IsA<FClassProperty>();
	case EOpenLogicUnderlyingType::Struct:
		return Property->IsA<FStructProperty>();
	default:
		return false;
	}
}
