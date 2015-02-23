// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollPlayerController.h"


ASteamrollPlayerController::ASteamrollPlayerController(const class FPostConstructInitializeProperties& PCIP)
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

