// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SlotsConfig.h"
#include "SlotContentConfig.generated.h"

USTRUCT(BlueprintType)
struct FSlotContentConfigStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	TEnumAsByte<ESlotTypeEnum::SlotType> SlotType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slots")
	int32 Quantity;

	FSlotContentConfigStruct()
	{
		SlotType = ESlotTypeEnum::SE_EMPTY;
		Quantity = 1;
	}
};


/**
 * Actor placed in a level to define what kind of slots and how many of them are available to the player
 */
UCLASS()
class STEAMROLL_API ASlotContentConfig : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TArray<FSlotContentConfigStruct> SlotContent;
	
};
