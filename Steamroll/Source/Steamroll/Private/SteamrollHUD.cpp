// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollHUD.h"
#include "Engine.h"


ASteamrollHUD::ASteamrollHUD(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	
}

void ASteamrollHUD::BeginPlay()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////So far only TSharedPtr<SMyUIWidget> has been created, now create the actual object.
	/////Create a SCustomizableInstanceProperties on heap, our MutableGameUIWidget shared pointer provides handle to object
	/////Widget will not self-destruct unless the HUD's SharedPtr (and all other SharedPtrs) destruct first.
	//SAssignNew(MutableGameUIWidget, SMutableGameHorizontalBox)
	//+ SHorizontalBox::Slot()
	//.MaxWidth(450)
	////.HAlign(HAlign_Center)
	//.VAlign(VAlign_Top)
	//[
	//	Aux		
	//];
}

//void ASteamrollHUD::Show() 
//{
//	MutableGameUIWidget->SetVisibility(EVisibility::Visible);
//}
//
//void ASteamrollHUD::Hide() 
//{
//	MutableGameUIWidget->SetVisibility(EVisibility::Hidden);
//}

void ASteamrollHUD::DrawHUD()
{
	Super::DrawHUD();
	////Reticle.SetupReticle(8.0f, 2.0f);

	//UFont* Font = GEngine->GetSmallFont();
	//// net mode
	//FString NetModeDesc =
	//	(GetNetMode() == NM_Client) ? TEXT("Client") :
	//	(GetNetMode() == NM_Standalone) ? TEXT("No network") : TEXT("Server");
	//DrawWithBackground(Font, NetModeDesc, FColor::Black, EAlignHorizontal::Right, 20, EAlignVertical::Top, 20);

	////AQAPawn* MyPawn = Cast<AQAPawn>(GetOwningPawn());
	////if (MyPawn && MyPawn->IsAlive())
	////{
	////	// health
	////	FString HealthDesc = FString::Printf(TEXT("Health: %d%%"), FMath::Trunc(100.0f * MyPawn->GetHealth() / MyPawn->GetMaxHealth()));
	////	FColor HealthColor(38, 128, 0);
	////	DrawWithBackground(Font, HealthDesc, HealthColor, EAlignHorizontal::Left, 20, EAlignVertical::Bottom, 20);

	////	// weapon
	////	DrawWeaponInfo();
	////}
	////else
	////{
	////	// respawn
	////	FString RespawnDesc = TEXT("Waiting for respawn");
	////	FColor RespawnColor(38, 128, 0);
	////	DrawWithBackground(Font, RespawnDesc, RespawnColor, EAlignHorizontal::Left, 20, EAlignVertical::Bottom, 20);
	////}

	//// engine version
	//FString VersionDesc = GEngineVersion.ToString();
	//DrawWithBackground(Font, VersionDesc, FColor::Black, EAlignHorizontal::Right, 20, EAlignVertical::Bottom, 20);
}

void ASteamrollHUD::DrawWithBackground(UFont* InFont, const FString& Text, const FColor& TextColor, EAlignHorizontal::Type HAlign, float OffsetX, EAlignVertical::Type VAlign, float OffsetY, float Alpha)
{
	float SizeX, SizeY;
	Canvas->StrLen(InFont, Text, SizeX, SizeY);

	const float PosX = (HAlign == EAlignHorizontal::Center) ? OffsetX + (Canvas->ClipX - SizeX) * 0.5f :
		(HAlign == EAlignHorizontal::Left) ? Canvas->OrgX + OffsetX :
		Canvas->ClipX - SizeX - OffsetX;
	const float PosY = (VAlign == EAlignVertical::Center) ? OffsetY + (Canvas->ClipY - SizeY) * 0.5f :
		(VAlign == EAlignVertical::Top) ? Canvas->OrgY + OffsetY :
		Canvas->ClipY - SizeY - OffsetY;

	const float BoxPadding = 5.0f;

	const float X = PosX - BoxPadding;
	const float Y = PosY - BoxPadding;
	const float Z = 0.1f;
	FCanvasTileItem TileItem(FVector2D(X, Y), FVector2D(SizeX + BoxPadding * 2.0f, SizeY + BoxPadding * 2.0f), FLinearColor(0.75f, 0.75f, 0.75f, Alpha));
	Canvas->DrawItem(TileItem);

	FLinearColor TextCol(TextColor);
	TextCol.A = Alpha;
	FCanvasTextItem TextItem(FVector2D(PosX, PosY), FText::FromString(Text), GEngine->GetSmallFont(), TextCol);
	Canvas->DrawItem(TextItem);
}

