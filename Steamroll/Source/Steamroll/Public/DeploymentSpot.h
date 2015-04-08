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
	TArray<TEnumAsByte<ESlotTypeEnum::SlotType>> ActivatorContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	float AimingStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	int32 NumSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	int32 NumBalls;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TArray<ACameraActor*> CameraList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	int32 CameraListActiveIndex;

	/** Used to control the launch power a base attached to this deployment spot, it's a factor that will multiply the lauch speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Base)
	float LaunchPowerFactor;

	/** Advance to the next camera in the camera list */
	UFUNCTION(BlueprintCallable, Category = Camera)
	void NextCamera();
	
};
