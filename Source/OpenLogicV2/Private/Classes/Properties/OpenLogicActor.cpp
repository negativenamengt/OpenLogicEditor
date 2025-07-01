// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicActor.h"
#include "Classes/Properties/OpenLogicObject.h"

UOpenLogicActor::UOpenLogicActor()
{
	PropertyDisplayName = FText::FromString("Actor");
	PropertyColor = FLinearColor(0.0, 0.2, 0.46, 1.0);
	SupportedAttributes = ActorMetaClass;
	UnderlyingType = EOpenLogicUnderlyingType::Object;
}