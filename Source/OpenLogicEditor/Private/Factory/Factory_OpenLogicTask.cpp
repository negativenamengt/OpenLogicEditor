// Copyright 2024 - NegativeNameSeller

#include "Factory/Factory_OpenLogicTask.h"
#include "Blueprint/Blueprint_OpenLogicTask.h"
#include "Tasks/OpenLogicTask.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"
#include "ClassViewerModule.h"
#include "Factory/Factory_OpenLogicGraph.h"

UOpenLogicTaskFactory::UOpenLogicTaskFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = UOpenLogicTaskBlueprint::StaticClass();
    ParentClass = UOpenLogicTask::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

bool UOpenLogicTaskFactory::ShouldShowInNewMenu() const
{
    return true;
}

bool UOpenLogicTaskFactory::ConfigureProperties()
{
    // Set up class viewer options
    FClassViewerInitializationOptions Options;
    Options.DisplayMode = EClassViewerDisplayMode::TreeView;
    Options.Mode = EClassViewerMode::ClassPicker;
    Options.bExpandRootNodes = true;

    Options.ExtraPickerCommonClasses.Add(UOpenLogicTask::StaticClass());

    // Set up the filter
    const TSharedPtr<FOpenLogicFilterViewer> Filter = MakeShareable(new FOpenLogicFilterViewer);
    Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;
    Options.ClassFilters.Add(Filter.ToSharedRef());
    Filter->AllowedChildrenOfClasses.Add(UOpenLogicTask::StaticClass());

    // Create the class picker
    UClass* ChosenClass = nullptr;
    const FText TitleText = FText::FromString("Pick a Open Logic Task Class");
    const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UOpenLogicTask::StaticClass());

    if (bPressedOk)
    {
        ParentClass = ChosenClass;
    }

    return bPressedOk;
}

UObject* UOpenLogicTaskFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
    if (ParentClass == nullptr || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
    {
        return nullptr;
    }

    UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, Name, BPTYPE_Normal, UOpenLogicTaskBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext);

    if (Blueprint && !Blueprint->UbergraphPages.IsEmpty())
    {
        int32 Position = 0;
        FKismetEditorUtilities::AddDefaultEventNode(Blueprint, Blueprint->UbergraphPages[0], GET_FUNCTION_NAME_CHECKED(UOpenLogicTask, OnTaskActivated), UOpenLogicTask::StaticClass(), Position);
        FKismetEditorUtilities::AddDefaultEventNode(Blueprint, Blueprint->UbergraphPages[0], GET_FUNCTION_NAME_CHECKED(UOpenLogicTask, OnTaskTick), UOpenLogicTask::StaticClass(), Position);
        FKismetEditorUtilities::AddDefaultEventNode(Blueprint, Blueprint->UbergraphPages[0], GET_FUNCTION_NAME_CHECKED(UOpenLogicTask, OnTaskCompleted), UOpenLogicTask::StaticClass(), Position);
    }

    return Blueprint;
}
