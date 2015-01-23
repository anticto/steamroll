// Copyright Anticto Estudi Binari, SL. All rights reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "SlotsConfig.h"
#include "Engine.h"
#include "SlotContentConfig.h"

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

	/** Gets a slot's param, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	float GetSlotParam(int32 SlotIndex, int32 ParamIndex);

	/** Sets a slot's param, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotParam(int32 SlotIndex, int32 ParamIndex, float Value);

	/** Gets a slot's connections, SlotIndex goes from 1 to 4, ConnectionIndex from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	bool GetSlotConnection(int32 SlotIndex, int32 ConnectionIndex);

	/** Sets a slot's param, SlotIndex goes from 1 to 4, ConnectionIndex from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotConnection(int32 SlotIndex, int32 ConnectionIndex, bool bIsConnected);

	// Radar and ball camera properties and functions
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	ASceneCapture2D* RadarCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	FTransform RadarTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	TArray<AActor*> BallCameras;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	int32 CurrentCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TArray<FSlotContentConfigStruct> SlotContent;

	UFUNCTION(BlueprintCallable, Category = Radar)
	void AddBallCamera(AActor* BallCamera);

	UFUNCTION(BlueprintCallable, Category = Radar)
	AActor* GetNextBallCamera();

	UFUNCTION(BlueprintCallable, Category = Radar)
	AActor* GetPrevBallCamera();

	UFUNCTION(BlueprintCallable, Category = Game)
	void DoGameOver();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Slots)
	void SpendItemsInSlots();

	UFUNCTION(BlueprintCallable, Category = Slots)
	void SpendSlotItem(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);
	
};
