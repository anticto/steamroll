// Copyright Anticto Estudi Binari, SL. All rights reserved.

#include "Steamroll.h"
#include "SteamrollPlayerController.h"


ASteamrollPlayerController::ASteamrollPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	WallDeploymentAngle = 0.f;

	CurrentCamera = -1;
}


TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollPlayerController::GetSlotState(int32 SlotIndex)
{
	return SlotsConfig.GetSlotType(SlotIndex);
}


void ASteamrollPlayerController::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.SetSlotType(SlotIndex, SlotTypeEnum);
}


float ASteamrollPlayerController::GetSlotParam(int32 SlotIndex, int32 ParamIndex)
{
	return SlotsConfig.GetSlotParam(SlotIndex, ParamIndex);
}


void ASteamrollPlayerController::SetSlotParam(int32 SlotIndex, int32 ParamIndex, float Value)
{
	SlotsConfig.SetSlotParam(SlotIndex, ParamIndex, Value);
}


bool ASteamrollPlayerController::GetSlotConnection(int32 SlotIndex, int32 ConnectionIndex)
{
	return SlotsConfig.GetSlotConnection(SlotIndex, ConnectionIndex);
}


void ASteamrollPlayerController::SetSlotConnection(int32 SlotIndex, int32 ConnectionIndex, bool bIsConnected)
{
	SlotsConfig.SetSlotConnection(SlotIndex, ConnectionIndex, bIsConnected);
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

