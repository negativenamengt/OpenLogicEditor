// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicTransform.h"

#include "Widgets/Input/SRotatorInputBox.h"
#include "Widgets/Input/SVectorInputBox.h"

UOpenLogicTransform::UOpenLogicTransform()
{
	PropertyDisplayName = FText::FromString("Transform");
	PropertyColor = FLinearColor(0.65, 0.12, 0.0, 1.0);
	UnderlyingType = EOpenLogicUnderlyingType::Struct;
	StructType = TBaseStructure<FTransform>::Get();
}