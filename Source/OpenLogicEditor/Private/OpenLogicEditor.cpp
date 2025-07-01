// Copyright 2024 - NegativeNameSeller

#include "OpenLogicEditor.h"
#include "Settings/OpenLogicEditorSettings.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Classes/OpenLogicGraphEditorManager.h"
#include "Styling/SlateStyle.h"
#include "EditorUtilitySubsystem.h"
#include "Customization/PropertyTypeCustomization.h"
#include "Framework/Notifications/NotificationManager.h"
#include "PinList/OpenLogicPinList.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FOpenLogicV2Module"

EAssetTypeCategories::Type FOpenLogicEditorModule::OpenLogic_AssetCategory = EAssetTypeCategories::Misc;
static inline TSharedPtr<FSlateStyleSet> OpenLogicStyleSet;

void FOpenLogicEditorModule::StartupModule()
{
	// Create the style set
	OpenLogicStyleSet = MakeShareable(new FSlateStyleSet("OpenLogicEditorStyle"));

	// Set the content root
	FString IconDir = FPaths::EngineContentDir() / TEXT("Editor/Slate/Starship/AssetIcons");
	OpenLogicStyleSet->SetContentRoot(IconDir);

	// Define the class thumbnails
	OpenLogicStyleSet->Set("ClassThumbnail.OpenLogicTask", new FSlateVectorImageBrush(OpenLogicStyleSet->RootToContentDir(TEXT("Blueprint_64.svg")), FVector2D(64.0f, 64.0f)));
	OpenLogicStyleSet->Set("ClassIcon.OpenLogicTask", new FSlateVectorImageBrush(OpenLogicStyleSet->RootToContentDir(TEXT("Blueprint_64.svg")), FVector2D(16.0f, 20.0f)));

	// Register the style set
	FSlateStyleRegistry::RegisterSlateStyle(*OpenLogicStyleSet);
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		OpenLogic_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName("Open Logic"), FText::FromName("Open Logic"));

		for (TSharedPtr<FAssetTypeActions_Base> AssetAction : OpenLogic_AssetActions)
		{
			AssetTools.RegisterAssetTypeActions(AssetAction.ToSharedRef());
		}
	}
	
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "OpenLogic_Settings",
			LOCTEXT("RuntimeSettingsName", "Open Logic Editor"), LOCTEXT("RuntimeSettingsDescription", "Configure Open Logic Editor settings"),
			GetMutableDefault<UOpenLogicEditorSettings>());
	}

	if (TaskGeneratorToolPath.IsValid() && GEditor)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->LoadedUIs.AddUnique(TaskGeneratorToolPath);
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnFilesLoaded().AddRaw(this, &FOpenLogicEditorModule::OnAssetRegistryFilesLoaded);

	// Bind the global class compilation event
	FCoreUObjectDelegates::OnObjectPostCDOCompiled.AddLambda([this](UObject* Object, const FObjectPostCDOCompiledContext& Context)
	{
		OnReloadComplete(Object, Context);
	});

	// Register the pin list factory
	FEdGraphUtilities::RegisterVisualPinFactory(MakeShareable(new FOpenLogicPinListFactory()));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(FOpenLogicPinData::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPinDataCustomization::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FOpenLogicEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		for (TSharedPtr<FAssetTypeActions_Base> AssetAction : OpenLogic_AssetActions)
		{
			AssetTools.UnregisterAssetTypeActions(AssetAction.ToSharedRef());
		}
	}
	
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "OpenLogic_Settings");
	}

	if (TaskGeneratorToolPath.IsValid() && GEditor)
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->LoadedUIs.Remove(TaskGeneratorToolPath);
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().OnFilesLoaded().RemoveAll(this);
	}

	FCoreUObjectDelegates::OnObjectPostCDOCompiled.RemoveAll(this);

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(FOpenLogicPinData::StaticStruct()->GetFName());
}

// Called when the asset registry has loaded all the files.
void FOpenLogicEditorModule::OnAssetRegistryFilesLoaded()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetData;

	const UClass* Class = UOpenLogicGraph::StaticClass();

	FTopLevelAssetPath ClassPath(*Class->GetPathName());
	AssetRegistry.GetAssetsByClass(ClassPath, AssetData);

	for (const FAssetData& Data : AssetData)
	{
		// Force the asset to load if it is not already loaded
		UObject* Asset = Data.GetAsset();
		if (!Asset)
		{
			Asset = LoadObject<UObject>(nullptr, *Data.GetObjectPathString());
		}

		// Check if the asset is a valid Open Logic Graph
		if (!Asset || !Asset->IsA(UOpenLogicGraph::StaticClass()))
		{
			continue;
		}

		// Convert the asset to the new format
		UOpenLogicGraph* Graph = Cast<UOpenLogicGraph>(Asset);

		int32 PreviousSchemaVersion = Graph->GraphData.SchemaVersion;
		Graph->MigrateToLatestSchemaVersion();

		if (PreviousSchemaVersion != Graph->GraphData.SchemaVersion)
		{
			// Save the asset if the schema version has changed
			Graph->MarkPackageDirty();

			// Add a notification to notify the user that the asset has been updated
			FNotificationInfo Info(FText::Format(LOCTEXT("SchemaVersionUpdated", "Converted Open Logic Graph '{0}' to schema version {1}"), FText::FromString(Graph->GetName()), FText::AsNumber(Graph->GraphData.SchemaVersion)));
			Info.ExpireDuration = 5.0f;
				
			FSlateNotificationManager::Get().AddNotification(Info);
		}
	}
}

void FOpenLogicEditorModule::OnReloadComplete(UObject* Object, const FObjectPostCDOCompiledContext& Context)
{
	if (Context.bIsRegeneratingOnLoad || Context.bIsSkeletonOnly || !Object->IsA(UUserWidget::StaticClass()))
	{
		return;
	}

	TSet<UGraphEditorBase*> GraphEditors = UOpenLogicGraphEditorManager::Get()->GetGraphEditors();

	for (UGraphEditorBase* GraphEditor : GraphEditors)
	{
		if (!IsValid(GraphEditor) || !GraphEditor->GetGraph())
		{
			continue;
		}
		
		GraphEditor->SetGraph(GraphEditor->GetGraph());

		// Log the graph editor that was reloaded
		UE_LOG(OpenLogicEditorLog, Log, TEXT("Graph Editor '%s' was reloaded."), *GraphEditor->GetPathName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOpenLogicEditorModule, OpenLogicEditor)
DEFINE_LOG_CATEGORY(OpenLogicEditorLog);