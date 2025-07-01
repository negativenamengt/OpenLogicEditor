// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicAny.h"

UOpenLogicAny::UOpenLogicAny()
{
	PropertyDisplayName = FText::FromString("Any");
	PropertyColor = FLinearColor(0.13, 0.12, 0.11);
	bResolvesTypeDynamically = true;
	UnderlyingType = EOpenLogicUnderlyingType::Wildcard;
}

bool UOpenLogicAny::IsCompatibleWith_Implementation(UOpenLogicProperty* OtherProperty)
{
	return true;
}