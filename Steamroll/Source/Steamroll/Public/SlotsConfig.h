

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

		//
		SE_Max		UMETA(Hidden),
	};
}


USTRUCT()
struct FSlotsConfigStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	TEnumAsByte<ESlotTypeEnum::SlotType> Slots[4];

	FSlotsConfigStruct()
	{
		for (uint32 i = 0; i < 4; ++i)
		{
			Slots[i] = ESlotTypeEnum::SE_EMPTY;
		}		
	}


	bool HasSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		for (uint32 i = 0; i < 4; ++i)
		{
			if (Slots[i] == SlotTypeEnum)
			{
				return true;
			}
		}

		return false;
	}

	int32 CountSlotState(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum)
	{
		int32 Count = 0;

		for (uint32 i = 0; i < 4; ++i)
		{
			if (Slots[i] == SlotTypeEnum)
			{
				Count++;
			}
		}

		return Count;
	}
};


/**
 * 
 */
UCLASS()
class STEAMROLL_API USlotsConfig : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum;
	
};
