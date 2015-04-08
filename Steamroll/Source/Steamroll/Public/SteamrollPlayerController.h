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

	/** The current ball configuration */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Slots)
	FSlotsConfigStruct SlotsConfig;

	/** The number of available slots in the current ball */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	int32 NumSlots;

	/** Gets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	TEnumAsByte<ESlotTypeEnum::SlotType> GetSlotState(int32 SlotIndex);
	
	/** Sets a slot's state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum);

	/** Sets a slot's complete state, SlotIndex goes from 1 to 4 */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SetSlotCompleteState(int32 SlotIndex, TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum, float Angle, TEnumAsByte<ESlotTypeEnum::SlotType> ActivatorType, float Time);

	// Radar and ball camera properties and functions
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	ASceneCapture2D* RadarCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	FTransform RadarTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	TArray<AActor*> BallCameras;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Radar)
	int32 CurrentCamera;

	/** The available items in the current deployment spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	TArray<FSlotContentConfigStruct> SlotContent;

	/** The debris in the level that should be deleted when there is too much */
	UPROPERTY()
	TArray<class ADebris*> LevelDebris;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Slots)
	float AimingStep;

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

	UFUNCTION(BlueprintCallable, Category = Slots)
	void IncrementSlotItem(TEnumAsByte<ESlotTypeEnum::SlotType> SlotTypeEnum, int32 Quantity);

	/** Checks whether the currently assembled steamball has all the required items in the current deployment spot */
	UFUNCTION(BlueprintCallable, Category = Slots)
	bool CheckItemAvailability();

	/** Checks whether the currently assembled steamball has all the required items in the current deployment spot */
	UFUNCTION(BlueprintCallable, Category = Slots)
	bool IsActivatorAvailable(TEnumAsByte<ESlotTypeEnum::SlotType> Activator);

	/** Returns true if there's more than one activator available in the current deployment spot */
	UFUNCTION(BlueprintCallable, Category = Slots)
	bool MorethanOneActivatorAvailable();

	/** Returns the number of available balls in the current deployment spot */
	UFUNCTION(BlueprintCallable, Category = Slots)
	int32 GetNumAvailableBalls() const;

	/** spends NumBalls balls in the current deployment spot */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void SpendBallsInDeploymentSpot(int32 NumBalls);

	/** Empties the currently assembled steamball */
	UFUNCTION(BlueprintCallable, Category = Slots)
	void EmptySteamball();

	/** Returns true if a 3dObject was clicked */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ui3D)
	bool LeftClickDown();

	/** Returns true if finishing a 3dui action, such as releasing a dragging action */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ui3D)
	bool LeftClickUp();
	
	/** Returns true if a 3dObject was clicked */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ui3D)
	bool RightClickDown();

	/** Returns true if finishing a 3dui action, such as releasing a dragging action */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Ui3D)
	bool RightClickUp();

	/** Converts Ui3D coords to world location and direction */
	UFUNCTION(BlueprintCallable, Category = Ui3D)
	void Ui3DToWorld(float X, float Y, float SizeX, float SizeY, const AActor* Camera, float FOV, FVector& OutLocation, FVector& OutDirection);

	UFUNCTION(BlueprintCallable, Category = Debris)
	void AddDebris(ADebris* DebrisActor);
};
