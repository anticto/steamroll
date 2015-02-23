// Copyright Anticto Estudi Binari, SL. All rights reserved.

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
	TEnumAsByte<ESlotTypeEnum::SlotType> ActivatorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	float Angle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	float Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	bool Used;

	FSlotStruct()
	{
		SlotType = ESlotTypeEnum::SE_EMPTY;
		ActivatorType = ESlotTypeEnum::SE_TIME;
		Angle = 0.0f;
		Time = 0.5f;

		Used = false;
	}
};


USTRUCT(BlueprintType)
struct FSlotsConfigStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TArray<FSlotStruct> Slots;

	FSlotsConfigStruct()
	{
		for (uint32 i = 0; i < 4; ++i)
		{
			Slots.Add(FSlotStruct(FSlotStruct()));
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


	/** Gets a slot's activator type, SlotIndex goes from 1 to 4 */
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotActivatorType(int32 SlotIndex)
	{
		return Slots[SlotIndex - 1].ActivatorType;
	}


	/** Sets a slot's activator type, SlotIndex goes from 1 to 4 */
	void SetSlotActivatorType(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		Slots[SlotIndex - 1].ActivatorType = SlotTypeEnum;
	}
	
	
	/** Gets a slot's angle, SlotIndex goes from 1 to 4 */
	float GetSlotAngle(int32 SlotIndex)
	{
		return Slots[SlotIndex - 1].Angle;
	}
	
	
	/** Sets a slot's angle, SlotIndex goes from 1 to 4 */
	void SetSlotAngle(int32 SlotIndex, float Value)
	{
		Slots[SlotIndex - 1].Angle = Value;
	}


	/** Gets a slot's time, SlotIndex goes from 1 to 4 */
	float GetSlotTime(int32 SlotIndex)
	{
		return Slots[SlotIndex - 1].Time;
	}


	/** Sets a slot's time, SlotIndex goes from 1 to 4 */
	void SetSlotTime(int32 SlotIndex, float Value)
	{
		Slots[SlotIndex - 1].Time = Value;
	}
	
	
	bool IsSlotUsed(int32 SlotIndex) const
	{
		return Slots[SlotIndex - 1].Used;
	}
	
	
	void SetSlotUsed(int32 SlotIndex, bool bUsed = true)
	{
		Slots[SlotIndex - 1].Used = bUsed;
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

