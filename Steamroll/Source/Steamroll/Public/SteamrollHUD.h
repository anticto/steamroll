// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/HUD.h"
#include "SteamrollHUD.generated.h"

namespace EAlignHorizontal
{
	enum Type
	{
		Left,
		Center,
		Right,
	};
}

namespace EAlignVertical
{
	enum Type
	{
		Top,
		Center,
		Bottom,
	};
}

/**
 * 
 */
UCLASS()
class ASteamrollHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Reference to an SCompoundWidget, TSharedPtr adds to the refcount of MyUIWidget
	/////MyUIWidget will not self-destruct as long as refcount > 0
	/////MyUIWidget refcount will be (refcout-1) if HUD is destroyed.
	//TSharedPtr<SMutableGameHorizontalBox> MutableGameUIWidget;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Called as soon as game starts, create SCompoundWidget and give Viewport access
	void BeginPlay();
	
	//void Show();
	//void Hide();

	virtual void DrawHUD() override;
	void DrawWithBackground(UFont* InFont, const FString& Text, const FColor& TextColor, EAlignHorizontal::Type HAlign, float OffsetX, EAlignVertical::Type VAlign, float OffsetY, float Alpha = 1.0f);
};
