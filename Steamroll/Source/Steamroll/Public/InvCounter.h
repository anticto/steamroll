// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "SlotsConfig.h"
#include "InvCounter.generated.h"

/**
 * Inventory Counter, with item and count display
 */
UCLASS()
class STEAMROLL_API AInvCounter : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	TEnumAsByte<ESlotTypeEnum::SlotType> ItemType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	UStaticMeshComponent* Module;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	UStaticMeshComponent* Item;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	class UChildActorComponent* NumberCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	USkeletalMeshComponent* CoverTop;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Counter)
	USkeletalMeshComponent* CoverBottom;

	void SetNumber(int32 Num);
};
