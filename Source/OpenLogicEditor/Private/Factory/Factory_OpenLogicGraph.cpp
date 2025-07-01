// Copyright 2024 - NegativeNameSeller

#include "Factory/Factory_OpenLogicGraph.h"
#include "Classes/OpenLogicGraph.h"
#include "ClassViewerModule.h"
#include "Kismet2/SClassPickerDialog.h"

UOpenLogicGraphFactory::UOpenLogicGraphFactory()
{
    SupportedClass = UOpenLogicGraph::StaticClass();
    bCreateNew = true;
}

UObject* UOpenLogicGraphFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    if (!SelectedClass || !SelectedClass->IsChildOf(UOpenLogicGraph::StaticClass()))
    {
        return nullptr;
    }

    UOpenLogicGraph* NewGraph = NewObject<UOpenLogicGraph>(InParent, SelectedClass, Name, Flags);
    NewGraph->GraphData.SchemaVersion = FOpenLogicGraphData::GetLatestSchemaVersion();
    
    return NewGraph;
}

bool UOpenLogicGraphFactory::ConfigureProperties()
{
    SelectedClass = nullptr;

    // Set up class viewer options
    FClassViewerInitializationOptions Options;
    Options.DisplayMode = EClassViewerDisplayMode::ListView;
    Options.Mode = EClassViewerMode::ClassPicker;

    Options.ExtraPickerCommonClasses.Add(UOpenLogicGraph::StaticClass());

    // Set up the filter
    const TSharedPtr<FOpenLogicFilterViewer> Filter = MakeShareable(new FOpenLogicFilterViewer);
    Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;
    Options.ClassFilters.Add(Filter.ToSharedRef());
    Filter->AllowedChildrenOfClasses.Add(UOpenLogicGraph::StaticClass());

    // Create the class picker
    UClass* ChosenClass = nullptr;
    const FText TitleText = FText::FromString("Pick a Open Logic Graph Class");
    const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UOpenLogicGraph::StaticClass());

    if (bPressedOk)
    {
        SelectedClass = ChosenClass;
    }

    return bPressedOk;
}