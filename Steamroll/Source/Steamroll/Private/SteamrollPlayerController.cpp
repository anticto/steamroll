

#include "Steamroll.h"
#include "SteamrollPlayerController.h"


ASteamrollPlayerController::ASteamrollPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	WallDeploymentAngle = 0.f;
}

TEnumAsByte<ESlotTypeEnum::SlotType> ASteamrollPlayerController::GetSlotState(int32 SlotIndex)
{
	return SlotsConfig.Slots[SlotIndex - 1];
}


void ASteamrollPlayerController::SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
{
	SlotsConfig.Slots[SlotIndex - 1] = SlotTypeEnum;
}


