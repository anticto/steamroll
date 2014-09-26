

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
	return SlotsConfig.Slots[SlotIndex - 1];
}


void ASteamrollPlayerController::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.Slots[SlotIndex - 1] = SlotTypeEnum;
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


