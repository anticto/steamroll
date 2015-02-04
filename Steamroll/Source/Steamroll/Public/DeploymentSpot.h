// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "SlotContentConfig.h"
#include "DeploymentSpot.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ADeploymentSpot : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TArray<FSlotContentConfigStruct> SlotContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	float AimingStep;
	
};
