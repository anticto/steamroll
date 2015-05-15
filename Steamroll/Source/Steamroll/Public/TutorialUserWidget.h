// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "TutorialUserWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDisplayDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConfirmDelegate, bool, Yes);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConfirmDelegate2, bool, Yes);

/**
 * 
 */
UCLASS(Abstract, EditInlineNew, BlueprintType, Blueprintable, Meta = (Category = "User Controls"))
class STEAMROLL_API UTutorialUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FDisplayDelegate DisplayDispatcher;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FConfirmDelegate ConfirmDispatcher;

	UFUNCTION(BlueprintCallable, Category = Dispatcher)
	void BindToDisplayDispatcher(const FTimerDynamicDelegate& Event);
	
	UFUNCTION(BlueprintCallable, Category = Dispatcher)
	void BindToConfirmDispatcher(const FConfirmDelegate2& Event);
};
