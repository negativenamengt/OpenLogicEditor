// Copyright 2024 - NegativeNameSeller

#include "Classes/Properties/OpenLogicObject.h"

#if WITH_EDITOR
#include "PropertyCustomizationHelpers.h"
#endif

UOpenLogicObject::UOpenLogicObject()
{
	PropertyDisplayName = FText::FromString("Object");
	PropertyColor = FLinearColor(0.0, 0.2, 0.46, 1.0);
	SupportedAttributes = MetaClass;
	UnderlyingType = EOpenLogicUnderlyingType::Object;
}

#if WITH_EDITOR
TArray<TSharedRef<SWidget>> UOpenLogicObject::CreateEditorInputWidgets(TSharedRef<FOpenLogicDefaultValueHandle> DefaultValueHandle, const FOpenLogicPinAttributes PinAttributes) const
{
	TSubclassOf<UObject> MetaClass = PinAttributes.MetaClass;

	// If the meta class is not set, default to UObject
	if (!MetaClass)
	{
		MetaClass = UObject::StaticClass();
	}
	
	if (SupportedAttributes == ActorMetaClass)
	{
		MetaClass = PinAttributes.ActorMetaClass;
	}

	TSoftObjectPtr<> Value = DefaultValueHandle->GetDefaultValue<TSoftObjectPtr<>>();

	TSharedPtr<FString> CachedObjectPath = MakeShared<FString>(Value->GetPathName());
	return { SNew(SObjectPropertyEntryBox)
		.AllowedClass(MetaClass)
		.AllowClear(false)
		.DisplayBrowse(true)
		.DisplayUseSelected(true)
		.ObjectPath_Lambda([CachedObjectPath]() { return *CachedObjectPath; })
		.OnObjectChanged_Lambda([DefaultValueHandle, CachedObjectPath](const FAssetData& AssetData) mutable
		{
			TSoftObjectPtr<> Object = AssetData.GetAsset();
			DefaultValueHandle->SetDefaultValue(Object);
			*CachedObjectPath = Object->GetPathName();
		})};
}
#endif