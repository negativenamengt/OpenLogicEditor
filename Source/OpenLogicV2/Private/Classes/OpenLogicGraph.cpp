// Copyright 2024 - NegativeNameSeller

#include "Classes/OpenLogicGraph.h"
#include "OpenLogicV2.h"
#include "Classes/GraphCustomization.h"
#include "Misc/OutputDeviceNull.h"

TSoftObjectPtr<UGraphCustomization> UOpenLogicGraph::GetDefaultGraphCustomization()
{
	static const TSoftObjectPtr<UGraphCustomization> DefaultCustomization =
		TSoftObjectPtr<UGraphCustomization>(FSoftObjectPath(TEXT("/OpenLogicV2/OpenLogicEditor/Styles/DefaultGraphStyle.DefaultGraphStyle")));

	return DefaultCustomization;
}

bool UOpenLogicGraph::FromString(FString Json)
{
	if (Json.IsEmpty())
	{
		return false;
	}
	
	const FProperty* Property = FindFProperty<FProperty>(GetClass(), GET_MEMBER_NAME_CHECKED(UOpenLogicGraph, GraphData));
	if (!Property)
	{
		return false;
	}
	
	const TCHAR* StringPtr = *Json;

	// Capture any import errors
	FOutputDeviceNull ErrorText;
	
	const TCHAR* Result = Property->ImportText_Direct(StringPtr, Property->ContainerPtrToValuePtr<void>(this), this, PPF_None, &ErrorText);
	if (!Result)
	{
		UE_LOG(OpenLogicLog, Error, TEXT("[FromString] Failed to parse serialized string."));
		return false;
	}

	return true;
}

FString UOpenLogicGraph::ToString()
{
	const FProperty* Property = FindFProperty<FProperty>(GetClass(), GET_MEMBER_NAME_CHECKED(UOpenLogicGraph, GraphData));

	FString StringValue;
	Property->ExportTextItem_Direct(StringValue, Property->ContainerPtrToValuePtr<void>(this), nullptr, nullptr, PPF_None);

	return StringValue;
}

bool UOpenLogicGraph::MigrateToLatestSchemaVersion()
{
	return GraphData.MigrateToLatestSchemaVersion();
}