

#pragma once

#include "GameFramework/PlayerController.h"
#include "SlotsConfig.h"
#include "Engine.h"

#include "SteamrollPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class STEAMROLL_API ASteamrollPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	AActor* LastDeployedActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	float WallDeploymentAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	FSlotsConfigStruct SlotsConfig;

	/** Gets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotState(int32 SlotIndex);
	
	/** Sets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);


	// Radar and ball camera properties and functions
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	ASceneCapture2D* RadarCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	FTransform RadarTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	TArray<AActor*> BallCameras;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	int32 CurrentCamera;

	UFUNCTION(BlueprintCallable, Category = Slots)
	void AddBallCamera(AActor* BallCamera);

	UFUNCTION(BlueprintCallable, Category = Slots)
	AActor* GetNextBallCamera();

	UFUNCTION(BlueprintCallable, Category = Slots)
	AActor* GetPrevBallCamera();
	
};
