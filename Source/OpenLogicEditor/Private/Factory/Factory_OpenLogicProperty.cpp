// Copyright 2024 - NegativeNameSeller

#include "Factory/Factory_OpenLogicProperty.h"

#include "ClassViewerModule.h"
#include "Blueprint/Blueprint_OpenLogicProperty.h"
#include "Factory/Factory_OpenLogicGraph.h"
#include "Tasks/OpenLogicProperty.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

UOpenLogicPropertyFactory::UOpenLogicPropertyFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = UOpenLogicPropertyBlueprint::StaticClass();
	ParentClass = UOpenLogicProperty::StaticClass();
	bCreateNew = true;
    bEditAfterNew = true;
}

bool UOpenLogicPropertyFactory::ShouldShowInNewMenu() const
{
	return true;
}

bool UOpenLogicPropertyFactory::ConfigureProperties()
{
	// Set up class viewer options
	FClassViewerInitializationOptions Options;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.bExpandRootNodes = true;

	Options.ExtraPickerCommonClasses.Add(UOpenLogicProperty::StaticClass());

	// Set up the filter
	const TSharedPtr<FOpenLogicFilterViewer> Filter = MakeShareable(new FOpenLogicFilterViewer);
	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;
	Options.ClassFilters.Add(Filter.ToSharedRef());
	Filter->AllowedChildrenOfClasses.Add(UOpenLogicProperty::StaticClass());

	// Create the class picker
	UClass* ChosenClass = nullptr;
	const FText TitleText = FText::FromString("Pick a Open Logic Property Class");
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UOpenLogicProperty::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UOpenLogicPropertyFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, UOpenLogicPropertyBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext);
}
