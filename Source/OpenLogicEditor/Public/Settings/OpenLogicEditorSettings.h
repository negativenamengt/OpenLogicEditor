// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Classes/GraphCustomization.h"
#include "Widgets/OpenLogicEditorBase.h"
#include "Widgets/InspectorBase.h"
#include "EditorUtilityWidget.h"
#include "OpenLogicEditorSettings.generated.h"

UCLASS(Config = Game, defaultconfig)
class OPENLOGICEDITOR_API UOpenLogicEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, NoClear, Category = "OpenLogicEditor", meta = (DisplayName = "Editor Base Class"))
		TSoftClassPtr<UOpenLogicEditorBase> EditorBaseGraphClass = TSoftClassPtr<UOpenLogicEditorBase>(FSoftObjectPath("/OpenLogicV2/OpenLogicEditor/OpenLogicEditor.OpenLogicEditor_C"));

	UPROPERTY(Config, EditAnywhere, NoClear, Category = "OpenLogicEditor")
		TSoftClassPtr<UInspectorBase> InspectorBaseClass = TSoftClassPtr<UInspectorBase>(FSoftObjectPath("/OpenLogicV2/OpenLogicEditor/OpenLogicInspector.OpenLogicInspector_C"));

public:
	UPROPERTY(Config, EditAnywhere, Category = "OpenLogicEditor")
		TArray<FString> PathsToScan = {"/Game/", "/OpenLogicV2/"};

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "OpenLogicEditor")
		bool AlwaysShowPluginChangelog = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "OpenLogicEditor")
		float AutoSaveInSeconds = 10;
};