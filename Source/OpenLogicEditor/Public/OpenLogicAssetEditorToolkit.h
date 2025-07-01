// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/OpenLogicEditorBase.h"

class FOpenLogicAssetEditorToolkit : public FAssetEditorToolkit
{
public:
	void InitEditor(const TArray<UObject*>& InObjects);

public:
	void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

public:
	FName GetToolkitFName() const override { return "OpenLogicEditor"; }
	FText GetBaseToolkitName() const override { return INVTEXT("Open Logic Editor"); }
	FString GetWorldCentricTabPrefix() const override { return "Open Logic "; }
	FLinearColor GetWorldCentricTabColorScale() const override { return {}; }

private:
	UPROPERTY()
		UObject* Graph;

	UPROPERTY()
		UOpenLogicEditorBase* OpenLogicEditorWidget;
};