// Copyright 2024 - NegativeNameSeller

#include "Widgets/OpenLogicPayloadEditor.h"
#include "DetailCategoryBuilder.h"

TSharedRef<IDetailCustomization> FOpenLogicPayloadEditorCustomization::MakeInstance()
{
	return MakeShareable(new FOpenLogicPayloadEditorCustomization);
}

void FOpenLogicPayloadEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<FName> CategoryNames;
	DetailBuilder.GetCategoryNames(CategoryNames);

	for (const FName& Category : CategoryNames)
	{
		IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory(Category);

		DetailCategory.InitiallyCollapsed(false);
	}
}

void UOpenLogicPayloadEditor::SetDefaultObject(UObject* InObject)
{
	DefaultObject = InObject;
}

void UOpenLogicPayloadEditor::SetNodes(const TArray<UNodeBase*>& InNodes)
{
	// Clear the task map.
	TaskMap.Empty();

	// Clear the task property names.
	PropertyNames.Empty();

	TaskMap.Reserve(InNodes.Num());

	for (UNodeBase* Node : InNodes)
	{
		if (!Node || !Node->TaskObject)
		{
			continue;
		}

		TaskMap.Add(Node->TaskObject, Node);

		// Retrieve the blueprint properties.
		TArray<FName> BlueprintProperties;
		Node->TaskObject->GetBlueprintTaskProperties().GenerateValueArray(BlueprintProperties);

		// Retrieve the C++ properties.
		TArray<FName> CppProperties = Node->TaskObject->GetCPPTaskProperties();

		// Reserve the space for the properties.
		PropertyNames.Reserve(BlueprintProperties.Num() + CppProperties.Num());
		
		// Add the properties to the property names.
		PropertyNames.Append(BlueprintProperties);
		PropertyNames.Append(CppProperties);
	}

	OnObjectsChanged();
}

TArray<UNodeBase*> UOpenLogicPayloadEditor::GetNodes() const
{
	TArray<UNodeBase*> Result;
	Result.Reserve(TaskMap.Num());

	TaskMap.GenerateValueArray(Result);

	return Result;
}

TArray<UOpenLogicTask*> UOpenLogicPayloadEditor::GetTaskObjects() const
{
	TArray<UOpenLogicTask*> Result;
	TaskMap.GenerateKeyArray(Result);

	return Result;
}

void UOpenLogicPayloadEditor::OnObjectsChanged()
{
	BuildPayloadEditor();
}

void UOpenLogicPayloadEditor::BuildPayloadEditor()
{
	DetailsView.Reset();

	if (!GetDisplayWidget().IsValid())
	{
		return;
	}

	if (!GIsEditor)
	{
		return SetMissingWidgetText(FText::FromString("Open Logic Payload Editor is editor-only."));
	}

	if (TaskMap.Num() == 0 && !IsValid(DefaultObject))
	{
		return SetMissingWidgetText(FText::FromString("No tasks to display."));
	}

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Setting the arguments for the details view.
	FDetailsViewArgs DetailsViewArgs;

	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bShowPropertyMatrixButton = false;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.bAllowSearch = bAllowSearch;

	// Creating the details view.
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// Adding the filters.
	DetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateUObject(this, &UOpenLogicPayloadEditor::GetIsPropertyVisible));

	// Customizing the details view.
	DetailsView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FOpenLogicPayloadEditorCustomization::MakeInstance));

	if (TaskMap.Num() == 0)
	{
		DetailsView->SetObject(DefaultObject);
	} else
	{
		DetailsView->SetObjects(TArray<UObject*>(GetTaskObjects()));
	}

	// Adding the details view to the display widget.
	if (DetailsView.IsValid())
	{
		GetDisplayWidget()->SetContent(DetailsView.ToSharedRef());
	}
}

void UOpenLogicPayloadEditor::SetMissingWidgetText(const FText& InText)
{
	if (!GetDisplayWidget().IsValid())
	{
		return;
	}

	GetDisplayWidget()->SetContent(
		SNew(STextBlock)
		.Text(InText)
	);
}

void UOpenLogicPayloadEditor::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	DetailsView.Reset();
	DisplayWidget.Reset();
}

const FText UOpenLogicPayloadEditor::GetPaletteCategory()
{
	return FText::FromString("OpenLogic");
}

TSharedRef<SWidget> UOpenLogicPayloadEditor::RebuildWidget()
{
	DisplayWidget = SNew(SBorder)
		.Padding(0.0f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.BorderImage(FAppStyle::GetBrush("NoBorder"));

	BuildPayloadEditor();

	return DisplayWidget.ToSharedRef();
}

void UOpenLogicPayloadEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	FNotifyHook::NotifyPostChange(PropertyChangedEvent, PropertyThatChanged);

	for (int i = 0; i < PropertyChangedEvent.GetNumObjectsBeingEdited(); i++)
	{
		const UObject* EditedObject = PropertyChangedEvent.GetObjectBeingEdited(i);

		const UOpenLogicTask* Task = Cast<UOpenLogicTask>(EditedObject);
		if (!Task)
		{
			continue;
		}

		// Get the node associated with the task.
		UNodeBase* Node = TaskMap.FindRef(Task);

		if (!Node)
		{
			continue;
		}

		// Update and save the task property.
		Node->SaveTaskProperty(PropertyThatChanged->GetFName());
		Node->SaveNode();

		// Broadcast the event to notify the node that a property has changed.
		Node->OnPayloadPropertyChanged.Broadcast(PropertyThatChanged->GetFName());
	}
	
}

bool UOpenLogicPayloadEditor::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	// Checks if the task map is empty.
	// If it is, it means that the default object is being displayed.
	if (TaskMap.IsEmpty())
	{
		const FName PropertyName = PropertyAndParent.Property.GetFName();

		// Hides the graph data and libraries properties. They are not supposed to be edited inside the payload editor.
		/*if (PropertyName == GET_MEMBER_NAME_CHECKED(UOpenLogicGraph, GraphData))
		{
			return false;
		}*/
		
		return true;
	}

	// Checks if the property is in the property names.
	return PropertyNames.Contains(PropertyAndParent.Property.GetFName());
}