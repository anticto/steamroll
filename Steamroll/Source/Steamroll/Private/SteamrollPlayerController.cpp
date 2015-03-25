// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollPlayerController.h"
#include "PlayerBase.h"
#include "DeploymentSpot.h"


ASteamrollPlayerController::ASteamrollPlayerController(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	WallDeploymentAngle = 0.f;

	CurrentCamera = -1;
	AimingStep = 22.5f;
}


TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollPlayerController::GetSlotState(int32 SlotIndex)
{
	return SlotsConfig.GetSlotType(SlotIndex);
}


void ASteamrollPlayerController::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.SetSlotType(SlotIndex, SlotTypeEnum);
}


void ASteamrollPlayerController::SetSlotCompleteState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum, float Angle, TEnumAsByte<ESlotTypeEnum::SlotType> ActivatorType, float Time)
{
	SlotsConfig.SetSlotType(SlotIndex, SlotTypeEnum);
	SlotsConfig.SetSlotAngle(SlotIndex, Angle);
	SlotsConfig.SetSlotActivatorType(SlotIndex, ActivatorType);
	SlotsConfig.SetSlotTime(SlotIndex, Time);
}


void ASteamrollPlayerController::AddBallCamera(AActor* BallCamera)
{
	BallCameras.Add(BallCamera);
}


AActor* ASteamrollPlayerController::GetNextBallCamera()
{
	int32& CameraIndex = CurrentCamera;

	if (CameraIndex == -1)
	{
		if (BallCameras.Num() <= 0)
		{
			return nullptr;
		}
		else 
		{
			CameraIndex = 0;
			return BallCameras[0];
		}
	}

	CameraIndex++;

	if (CameraIndex >= BallCameras.Num())
	{
		CameraIndex = 0;
	}

	return BallCameras[CameraIndex];
}


AActor* ASteamrollPlayerController::GetPrevBallCamera()
{
	int32& CameraIndex = CurrentCamera;

	if (CameraIndex == -1)
	{
		if (BallCameras.Num() <= 0)
		{
			return nullptr;
		}
		else
		{
			CameraIndex = BallCameras.Num() - 1;
			return BallCameras[CameraIndex];
		}
	}

	CameraIndex--;

	if (CameraIndex < 0)
	{
		CameraIndex = BallCameras.Num() - 1;
	}

	return BallCameras[CameraIndex];
}


void ASteamrollPlayerController::DoGameOver()
{
	//FString MapName = GetWorld()->GetMapName();

	//if (MapName != nullptr)
	//{
	//	GetWorld()->ServerTravel(MapName);
	//}
}


void ASteamrollPlayerController::SpendItemsInSlots_Implementation()
{

}


void ASteamrollPlayerController::SpendSlotItem(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	for (auto& Slot : SlotContent)
	{
		if (Slot.SlotType == SlotTypeEnum && Slot.Quantity > 0)
		{
			Slot.Quantity--;
			break;
		}
	}
}


bool ASteamrollPlayerController::CheckItemAvailability()
{
	auto SlotContentCopy = SlotContent;

	for (int32 i = 1; i < 5; ++i)
	{
		auto NeededType = SlotsConfig.GetSlotType(i);

		if (NeededType == ESlotTypeEnum::SE_EMPTY)
		{
			continue;
		}

		bool bFoundNeededQuantity = false;

		for (auto& Slot : SlotContentCopy)
		{
			if (Slot.SlotType == NeededType && Slot.Quantity > 0)
			{
				Slot.Quantity--;
				bFoundNeededQuantity = true;
				break;
			}
		}

		if (!bFoundNeededQuantity)
		{
			return false;
		}
	}

	return true;
}


void ASteamrollPlayerController::IncrementSlotItem(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum, int32 Quantity)
{
	bool bFoundItem = false;

	for (auto& Slot : SlotContent)
	{
		if (Slot.SlotType == SlotTypeEnum)
		{
			Slot.Quantity += Quantity;
			bFoundItem = true;
			break;
		}
	}

	if (!bFoundItem)
	{
		FSlotContentConfigStruct ConfigStruct;
		ConfigStruct.SlotType = SlotTypeEnum;
		ConfigStruct.Quantity = Quantity;
		SlotContent.Add(ConfigStruct);
	}
}


bool ASteamrollPlayerController::LeftClickDown_Implementation()
{
	return false;
}


bool ASteamrollPlayerController::LeftClickUp_Implementation()
{
	return false;
}


bool ASteamrollPlayerController::RightClickDown_Implementation()
{
	return false;
}


bool ASteamrollPlayerController::RightClickUp_Implementation()
{
	return false;
}


void ASteamrollPlayerController::EmptySteamball()
{
	for (int32 i = 1; i < 4; ++i)
	{
		SlotsConfig.SetSlotType(i, ESlotTypeEnum::SE_EMPTY);
		SlotsConfig.SetSlotActivatorType(i, ESlotTypeEnum::SE_TIME);
		SlotsConfig.SetSlotAngle(i, 0.f);
		SlotsConfig.SetSlotTime(i, 5.f);
		SlotsConfig.SetSlotUsed(i, false);
	}
}


void ASteamrollPlayerController::Ui3DToWorld(float X, float Y, float SizeX, float SizeY, const AActor* Camera, float FOV, FVector& OutLocation, FVector& OutDirection)
{
	float CenteredX = X - SizeX / 2.f;
	float CenteredY = Y - SizeY / 2.f;

	//UE_LOG(LogTemp, Error, TEXT("%f; %f"), CenteredX, CenteredY);
	
	FVector Origin = Camera->GetActorLocation() + Camera->GetActorForwardVector();
	float UnitLength = FMath::Tan(FOV / 2.f) / (SizeY / 1.f);

	FVector Vx = Camera->GetActorRightVector() * UnitLength;
	FVector Vy = -Camera->GetActorUpVector() * UnitLength;
	//DrawDebugLine(GetWorld(), Origin, Origin + Vx.GetSafeNormal() * 1000.f, FColor::Red, false, 3.f);
	//DrawDebugLine(GetWorld(), Origin, Origin + Vy.GetSafeNormal() * 1000.f, FColor::Blue, false, 3.f);

	OutLocation = Origin + Vx * CenteredX + Vy * CenteredY;
	OutDirection = (OutLocation - Camera->GetActorLocation()).GetSafeNormal();
}


bool ASteamrollPlayerController::IsActivatorAvailable(TEnumAsByte<ESlotTypeEnum::SlotType> Activator)
{
	APlayerBase* Base = Cast<APlayerBase>(GetPawn());

	if (Base)
	{
		if (Base->AttachedToDeploymentSpot->ActivatorContent.Num() == 0)
		{
			return true;
		}

		bool AllEmpty = true;

		for (auto& ActivatorItem : Base->AttachedToDeploymentSpot->ActivatorContent)
		{
			if (Activator == ActivatorItem)
			{
				return true;
			}

			if (ActivatorItem != ESlotTypeEnum::SE_EMPTY)
			{
				AllEmpty = false;
			}
		}

		if (AllEmpty)
		{
			return true;
		}
	}

	return false;
}


bool ASteamrollPlayerController::MorethanOneActivatorAvailable()
{
	APlayerBase* Base = Cast<APlayerBase>(GetPawn());

	if (Base)
	{
		if (Base->AttachedToDeploymentSpot->ActivatorContent.Num() == 1)
		{
			return false;
		}
	}

	return true;
}

