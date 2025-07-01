// Copyright 2024 - NegativeNameSeller

#include "OpenLogicAssetEditorToolkit.h"
#include "Classes/OpenLogicGraph.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/InspectorBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Settings/OpenLogicEditorSettings.h"
#include "Engine/World.h"

void FOpenLogicAssetEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
    // Cast the first object in the array to UOpenLogicGraphObject
    Graph = Cast<UOpenLogicGraph>(InObjects[0]);

    // Define the layout of the editor using FTabManager
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("OpenLogicEditorLayout")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Vertical)

            ->Split
            (
                FTabManager::NewSplitter()
                ->SetSizeCoefficient(0.6f)
                ->SetOrientation(Orient_Horizontal)
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.8f)
                    ->AddTab("Graph View", ETabState::OpenedTab)
                )

                // Add a new splitter and stack on the right
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetSizeCoefficient(0.5f)
                    ->SetOrientation(Orient_Horizontal)
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.8f)
                        ->AddTab("Inspector View", ETabState::OpenedTab)
                    )
                )
            )
        );

    // Initialize the asset editor with the defined layout
    FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, {}, "OpenLogicEditor", Layout, true, true, InObjects);
}

void FOpenLogicAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("Open Logic Editor"));

    UWorld* World = GEditor->GetEditorWorldContext().World();
    check(World);

    // Retrieving the editor base class and the inspector base class from the editor settings
    UOpenLogicEditorSettings* Settings = GetMutableDefault<UOpenLogicEditorSettings>();

    TSubclassOf<UEditorUtilityWidget> GraphClass = Settings->EditorBaseGraphClass.LoadSynchronous();
    check(GraphClass);

    TSubclassOf<UInspectorBase> InspectorClass = Settings->InspectorBaseClass.LoadSynchronous();
    check(InspectorClass);

    OpenLogicEditorWidget = CreateWidget<UOpenLogicEditorBase>(World, GraphClass);
    OpenLogicEditorWidget->GraphObject = Cast<UOpenLogicGraph>(Graph);
    OpenLogicEditorWidget->NodeInspectorWidget = CreateWidget<UInspectorBase>(World, InspectorClass);

    // Register the Graph View tab
    InTabManager->RegisterTabSpawner("Graph View", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs&)
        {
            TSharedRef<SDockTab> DockTab = SNew(SDockTab);
            DockTab.Get().SetContent(OpenLogicEditorWidget->TakeWidget());
            return DockTab;
        }))
        .SetDisplayName(INVTEXT("Graph View"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());

    // Register the Node Inspector tab
    InTabManager->RegisterTabSpawner("Inspector View", FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs&)
        {
            TSharedRef<SDockTab> DockTab = SNew(SDockTab);
            DockTab.Get().SetContent(OpenLogicEditorWidget->NodeInspectorWidget->TakeWidget());
            return DockTab;
        }))
        .SetDisplayName(INVTEXT("Inspector View"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FOpenLogicAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner("Graph View");
    InTabManager->UnregisterTabSpawner("Inspector View");
}