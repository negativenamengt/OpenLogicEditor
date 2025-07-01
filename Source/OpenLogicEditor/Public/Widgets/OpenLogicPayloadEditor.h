// Copyright 2024 - NegativeNameSeller

#pragma once

#include "CoreMinimal.h"
#include "IDetailsView.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/NodeBase.h"
#include "OpenLogicPayloadEditor.generated.h"

UCLASS(Blueprintable)
class OPENLOGICEDITOR_API UOpenLogicPayloadEditor : public UWidget, public FNotifyHook
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void SetDefaultObject(UObject* InObject);
	
	UFUNCTION(BlueprintCallable, Category = "OpenLogic")
		void SetNodes(const TArray<UNodeBase*>& InNodes);

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		TArray<UNodeBase*> GetNodes() const;

	UFUNCTION(BlueprintPure, Category = "OpenLogic")
		TArray<UOpenLogicTask*> GetTaskObjects() const;

public:
	UPROPERTY(EditAnywhere, Category = "View")
		bool bAllowSearch = true;

protected:
	// The object that will be inspected when no nodes are specified.
	UPROPERTY()
		UObject* DefaultObject;
	
	UPROPERTY()
		TMap<UOpenLogicTask*, UNodeBase*> TaskMap;

	UPROPERTY()
		TSet<FName> PropertyNames;

private:
	void OnObjectsChanged();

protected:
	void BuildPayloadEditor();
	void SetMissingWidgetText(const FText& InText);
	TSharedPtr<SBorder> GetDisplayWidget() const { return DisplayWidget; }

public:
	//~ UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual const FText GetPaletteCategory() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget interface

protected:
	//~ FNotifyHook interface
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End of FNotifyHook interface

protected:
	virtual bool GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const;

private:
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SBorder> DisplayWidget;
};

class FOpenLogicPayloadEditorCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};