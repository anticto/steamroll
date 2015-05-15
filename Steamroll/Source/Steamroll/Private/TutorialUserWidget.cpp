// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "TutorialUserWidget.h"


void UTutorialUserWidget::BindToDisplayDispatcher(const FTimerDynamicDelegate& Event)
{
	DisplayDispatcher.Add(Event);
}


void UTutorialUserWidget::BindToConfirmDispatcher(const FConfirmDelegate2& Event)
{
	ConfirmDispatcher.Add(Event);
}

