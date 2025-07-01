// Copyright 2024 - NegativeNameSeller

#include "Widgets/OpenLogicPayloadWidget.h"
#include "Widgets/NodeBase.h"

void UDEPRECATED_OpenLogicPayloadWidget::OnPayloadInitialized_Implementation()
{}

void UDEPRECATED_OpenLogicPayloadWidget::AlertPropertyChange(FName PropertyName)
{
	if (!NodeWidget)
	{
		return;
	}

	NodeWidget->OnPayloadPropertyChanged.Broadcast(PropertyName);
}
