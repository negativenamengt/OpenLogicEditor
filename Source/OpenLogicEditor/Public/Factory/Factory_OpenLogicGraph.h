// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "ClassViewerFilter.h"
#include "Factories/Factory.h"
#include "Factory_OpenLogicGraph.generated.h"

UCLASS()
class OPENLOGICEDITOR_API UOpenLogicGraphFactory : public UFactory
{
	GENERATED_BODY()
public:
	UOpenLogicGraphFactory();

public:
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ConfigureProperties() override;

protected:
	UPROPERTY()
		TSubclassOf<UObject> SelectedClass;
};

// This class is used to filter the class viewer to only show classes that are children of UOpenLogicGraphObject
class FOpenLogicFilterViewer : public IClassViewerFilter
{
public:
	TSet<const UClass*> AllowedChildrenOfClasses;
	EClassFlags DisallowedClassFlags;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};