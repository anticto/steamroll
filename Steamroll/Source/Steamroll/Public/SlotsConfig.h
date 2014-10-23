

#pragma once

#include "Object.h"
#include "SlotsConfig.generated.h"


UENUM(BlueprintType)		//"BlueprintType" is essential to include
namespace ESlotTypeEnum
{
	//Max of 256 entries per type, each entry gets associated with 0-255
	enum SlotType
	{
		SE_EMPTY 	UMETA(DisplayName = "Empty"),
		SE_WALL 	UMETA(DisplayName = "Wall"),
		SE_EXPL		UMETA(DisplayName = "Explosion"),
		SE_RAMP 	UMETA(DisplayName = "Ramp"),
		SE_TIME 	UMETA(DisplayName = "Timer"),
		SE_REMOTE 	UMETA(DisplayName = "Remote Trigger"),
		SE_CONTACT 	UMETA(DisplayName = "Contact Trigger"),
		SE_IMP	 	UMETA(DisplayName = "Impulse"),
		SE_LIGHT	UMETA(DisplayName = "Light"),
		SE_CAMERA	UMETA(DisplayName = "Camera"),
		SE_PORTAL	UMETA(DisplayName = "Portal"),
		SE_LAUNCH	UMETA(DisplayName = "Launcher"),
		SE_COLLECT	UMETA(DisplayName = "Collect"),
		SE_PAINT	UMETA(DisplayName = "Paint"),
		SE_STOP 	UMETA(DisplayName = "Stop Trigger"),

		//
		SE_Max		UMETA(Hidden),
	};
}


USTRUCT(BlueprintType)
struct FSlotStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	TEnumAsByte<ESlotTypeEnum::SlotType> SlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	float SlotParam1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	float SlotParam2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	TArray<bool> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	bool Used;

	FSlotStruct(TEnumAsByte<ESlotTypeEnum::SlotType> SlotType = ESlotTypeEnum::SE_EMPTY, float SlotParam1 = 0.5f, float SlotParam2 = 0.5f)
	{
		this->SlotType = SlotType;
		this->SlotParam1 = SlotParam1;
		this->SlotParam2 = SlotParam2;

		for (uint32 i = 0; i < 4; ++i)
		{
			Connections.Add(true);
		}

		Used = false;
	}
};


USTRUCT(BlueprintType)
struct FSlotsConfigStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	TArray<FSlotStruct> Slots;

	FSlotsConfigStruct()
	{
		for (uint32 i = 0; i < 4; ++i)
		{
			Slots.Add(FSlotStruct(ESlotTypeEnum::SE_EMPTY));
		}		
	}


	/** Gets a slot's state, SlotIndex goes from 1 to 4 */
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotType(int32 SlotIndex)
	{
		return Slots[SlotIndex - 1].SlotType;
	}


	/** Sets a slot's state, SlotIndex goes from 1 to 4 */
	void SetSlotType(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		Slots[SlotIndex - 1].SlotType = SlotTypeEnum;
	}


	/** Gets a slot's param, SlotIndex goes from 1 to 4, ParamIndex from 1 to 2 */
	float GetSlotParam(int32 SlotIndex, int32 ParamIndex)
	{
		return ParamIndex == 2 ? Slots[SlotIndex - 1].SlotParam2 : Slots[SlotIndex - 1].SlotParam1;
	}


	/** Sets a slot's param, SlotIndex goes from 1 to 4, ParamIndex from 1 to 2 */
	void SetSlotParam(int32 SlotIndex, int32 ParamIndex, float Value)
	{
		if (ParamIndex == 2)
		{
			Slots[SlotIndex - 1].SlotParam2 = Value;
		}
		else
		{
			Slots[SlotIndex - 1].SlotParam1 = Value;
		}
	}

	/** Gets a slot's connections, SlotIndex goes from 1 to 4, ConnectionIndex from 1 to 4 */
	bool GetSlotConnection(int32 SlotIndex, int32 ConnectionIndex)
	{
		return Slots[SlotIndex - 1].Connections[ConnectionIndex - 1];
	}


	/** Sets a slot's param, SlotIndex goes from 1 to 4, ConnectionIndex from 1 to 4 */
	void SetSlotConnection(int32 SlotIndex, int32 ConnectionIndex, bool bIsConnected)
	{
		Slots[SlotIndex - 1].Connections[ConnectionIndex - 1] = bIsConnected;
	}


	bool IsSlotUsed(int32 SlotIndex) const
	{
		return Slots[SlotIndex - 1].Used;
	}


	void SetSlotUsed(int32 SlotIndex)
	{
		Slots[SlotIndex - 1].Used = true;
	}


	bool HasSlotType(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		for (uint32 i = 0; i < 4; ++i)
		{
			if (Slots[i].SlotType == SlotTypeEnum)
			{
				return true;
			}
		}

		return false;
	}


	int32 CountSlotType(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		int32 Count = 0;

		for (uint32 i = 0; i < 4; ++i)
		{
			if (Slots[i].SlotType == SlotTypeEnum)
			{
				Count++;
			}
		}

		return Count;
	}
};

